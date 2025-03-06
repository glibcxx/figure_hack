#include "CircuitPendingUpdateVisualization.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ThreadPoolExecutor.h>
#include <mc/world/level/Level.h>
#include <mc/world/redstone/circuit/CircuitSceneGraph.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>
#include <mc/world/redstone/circuit/CircuitTrackingInfo.h>
#include <mc/world/redstone/circuit/components/BaseRailTransporter.h>
#include <mc/world/redstone/circuit/components/PoweredBlockComponent.h>
#include <mc/world/redstone/circuit/components/TransporterComponent.h>

#include "figure_hack/Utils/BlockHighlight.h"
#include "figure_hack/figure_hack.h"

namespace fh {

std::unordered_map<BlockPos, DimensionType> posToDebug;

ll::thread::ThreadPoolExecutor rsThread{"Redstone Thread", 1};

std::thread::id rsThreadId;

bool rsThreadStarted = false;

std::condition_variable cv;
std::mutex              mutex_cv;
bool                    rs = false;

bool CPUVisualize::switchPos(BlockSource& region, const BlockPos& pos) {
    auto found = posToDebug.find(pos);
    bool ret   = false;
    if (found == posToDebug.end()) {
        BlockHighlightManager::addStable(region, pos, BlockHighlightManager::Color::gray);
        posToDebug.emplace(pos, region.getDimensionId());
        ret = true;
    } else {
        BlockHighlightManager::removeStable(region, pos);
        posToDebug.erase(pos);
        ret = false;
    }
    return ret;
}

bool CPUVisualize::tryRemovePos(BlockSource& region, const BlockPos& pos) {
    auto found = posToDebug.find(pos);
    if (found != posToDebug.end() && found->second == region.getDimensionId()) {
        BlockHighlightManager::removeStable(region, pos);
        posToDebug.erase(pos);
        return true;
    }
    return false;
}

void CPUVisualize::clearPos(Level& level) {
    for (auto&& [pos, dimType] : posToDebug) {
        auto ref = level.getDimension(dimType);
        if (!ref.expired()) {
            BlockHighlightManager::removeStable(ref.lock()->getBlockSourceFromMainChunkSource(), pos);
        }
    }
    posToDebug.clear();
}

BlockSource* currentRegion = nullptr;

const uint32_t SPEED = 2;
uint32_t       now   = 0;

LL_AUTO_TYPE_INSTANCE_HOOK( // NOLINT
    ProcessPendingUpdates_Hook,
    ll::memory::HookPriority::Normal,
    CircuitSceneGraph,
    &CircuitSceneGraph::update,
    void,
    BlockSource* region
) {
    if (figureHack::getInstance().getConfig().function.enable_microtick) {
        if (rsThreadStarted && currentRegion == region) {
            if ((now % SPEED) == 0) {
                now = 0;
                std::unique_lock lock(mutex_cv);
                cv.wait(lock, []() { return !rs; });
                rs = true;
                cv.notify_all();
            }
            now++;
            return;
        }
    }
    this->origin(region);
}

LL_AUTO_TYPE_INSTANCE_HOOK( // NOLINT
    FindRelationships_Hook,
    ll::memory::HookPriority::Normal,
    CircuitSceneGraph,
    &CircuitSceneGraph::findRelationships,
    void,
    const BlockPos&       pos,
    BaseCircuitComponent* producerTarget,
    BlockSource*          region
) {
    using namespace std::chrono_literals;
    if (rsThreadStarted) {
        this->origin(pos, producerTarget, region);
    } else if (CPUVisualize::tryRemovePos(*region, pos)) {
        currentRegion   = region;
        rsThreadStarted = true;
        rsThread.execute([this, &pos, producerTarget, region]() {
            rsThreadId = std::this_thread::get_id();
            {
                std::unique_lock lock(mutex_cv);
                cv.wait(lock, []() { return rs; });
            }
            this->origin(pos, producerTarget, region);
            std::unique_lock lock(mutex_cv);
            rs              = false;
            rsThreadStarted = false;
            currentRegion   = nullptr;
            cv.notify_all();
            cv.wait(lock);
        });
    } else {
        this->origin(pos, producerTarget, region);
    }
}

void setCircuitTrackingInfo(
    CircuitTrackingInfo::Entry& old,
    BaseCircuitComponent*       component,
    const BlockPos&             pos,
    const uchar&                direction,
    const CircuitComponentType  typeID
) {
    old.mComponent = component;
    old.mPos       = pos;
    old.mDirection = direction;
    old.mTypeID    = typeID;
}

int addToFillQueue(
    ::CircuitSceneGraph&               graph,
    ::CircuitComponentList&            powerAssociationMap,
    ::BaseCircuitComponent*            newComponent,
    ::CircuitTrackingInfo&             info,
    const BlockPos&                    newPos,
    schar                              face,
    std::queue<::CircuitTrackingInfo>& queue
) {
    if (!newComponent) return -1;
    ::CircuitTrackingInfo newInfo = info;
    CircuitComponentType  type    = newComponent->getCircuitComponentGroupType();
    size_t                oldSize = newComponent->mSources->mComponents.size();
    setCircuitTrackingInfo(newInfo.mCurrent, newComponent, newPos, face, type);
    int  newDampening     = info.mDampening;
    bool bDirectlyPowered = info.mDirectlyPowered;
    powerAssociationMap.mComponents.emplace_back(newComponent, 0, newPos);
    if (info.mNearest->mComponent->allowConnection(graph, newInfo, bDirectlyPowered)) {
        if (newComponent->addSource(graph, newInfo, newDampening, bDirectlyPowered)) {
            newInfo.m2ndNearest = info.mNearest;
            setCircuitTrackingInfo(newInfo.mNearest, newComponent, newPos, face, newInfo.mCurrent->mTypeID);
            newInfo.mDampening       = newDampening;
            newInfo.mDirectlyPowered = bDirectlyPowered;
            queue.push(newInfo);
            return true;
        } else if (oldSize != newComponent->mSources->mComponents.size()) {
            return true;
        }
    }
    return false;
}


LL_AUTO_STATIC_HOOK( // NOLINT
    addToFillQueue_Hook,
    ll::memory::HookPriority::Normal,
    ll::memory::SymbolView{"addToFillQueue"},
    void,
    ::CircuitSceneGraph&               graph,
    ::CircuitComponentList&            powerAssociationMap,
    ::BaseCircuitComponent*            newComponent,
    ::CircuitTrackingInfo&             info,
    const ::BlockPos&                  newPos,
    schar                              face,
    std::queue<::CircuitTrackingInfo>& queue
) {
    using namespace std::chrono_literals;
    if (rsThreadStarted && std::this_thread::get_id() == rsThreadId) {
        BlockHighlightManager::add(
            *currentRegion,
            info.mNearest->mPos,
            {.color = BlockHighlightManager::Color::silver, .lifespan = SPEED}
        );
        int res = addToFillQueue(graph, powerAssociationMap, newComponent, info, newPos, face, queue);
        switch (res) {
        case 0:
            BlockHighlightManager::add(
                *currentRegion,
                newPos,
                {.color = BlockHighlightManager::Color::red, .lifespan = SPEED}
            );
            break;
        case 1:
            BlockHighlightManager::add(
                *currentRegion,
                newPos,
                {.color = BlockHighlightManager::Color::lime, .lifespan = SPEED}
            );
            break;
        case -1:
        default:
            BlockHighlightManager::add(
                *currentRegion,
                newPos,
                {.color = BlockHighlightManager::Color::yellow, .lifespan = SPEED}
            );
            break;
        }
        std::unique_lock lock(mutex_cv);
        rs = false;
        cv.notify_all();
        cv.wait(lock, []() { return rs; });
    } else {
        origin(graph, powerAssociationMap, newComponent, info, newPos, face, queue);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK( // NOLINT
    Evaluate_Hook,
    ll::memory::HookPriority::Normal,
    CircuitSystem,
    &CircuitSystem::evaluate,
    void,
    BlockSource* region
) {
    if (rsThreadStarted) return;
    this->origin(region);
}

// 以下函数以及上面的addToFillQueue为电路搜索算法的完整复原，以应对addToFillQueue被完全内联的情况

// void searchForRelationshipAt(
//     ::BlockSource*                     bs,
//     ::CircuitComponentList&            powerAssociationMap,
//     ::CircuitSceneGraph&               graph,
//     ::CircuitTrackingInfo&             trackInfo,
//     schar                              facing,
//     std::queue<::CircuitTrackingInfo>& queue
// ) {
//     BlockPos              newPos       = trackInfo.mNearest.mPos + Facing::DIRECTION[facing];
//     BaseCircuitComponent* newComponent = graph.getBaseComponent(newPos);
//     if (newPos != trackInfo.mNearest.mPos && newPos != trackInfo.m2ndNearest.mPos) {
//         addToFillQueue(graph, powerAssociationMap, newComponent, trackInfo, newPos, facing, queue);
//     }
// }

// void checkComponent(
//     ::CircuitSceneGraph&               graph,
//     ::CircuitComponentList&            powerAssociationMap,
//     schar                              facingId,
//     const ::BlockPos&                  otherPos,
//     ::CircuitTrackingInfo&             info,
//     std::queue<::CircuitTrackingInfo>& positions,
//     bool                               goingDown,
//     ::CircuitComponentType             type
// ) {
//     BlockPos              targetPos = otherPos + Facing::DIRECTION[facingId];
//     BaseCircuitComponent* Component = graph.getComponent(targetPos, type);
//     addToFillQueue(graph, powerAssociationMap, Component, info, targetPos, facingId, positions);
// }

// void searchForRelationhshipsAboveAndBelow(
//     ::CircuitSceneGraph&               graph,
//     ::CircuitComponentList&            powerAssociationMap,
//     ::BaseCircuitComponent*            currentComponent,
//     ::BlockPos                         targetPos,
//     ::CircuitTrackingInfo&             info,
//     ::FacingID                         dir,
//     std::queue<::CircuitTrackingInfo>& positions
// ) {
//     auto typeID = currentComponent->getCircuitComponentGroupType();
//     if (typeID == CircuitComponentType::TransporterComponent) {
//         BlockPos              otherPos = targetPos + Facing::DIRECTION[1];
//         BaseCircuitComponent* up       = graph.getComponent(otherPos, CircuitComponentType::PoweredBlockComponent);
//         if (!up || up->mAllowPowerUp) {
//             for (schar facing = 2; facing <= 5; facing++) {
//                 checkComponent(
//                     graph,
//                     powerAssociationMap,
//                     facing,
//                     otherPos,
//                     info,
//                     positions,
//                     false,
//                     CircuitComponentType::TransporterComponent
//                 );
//             }
//         }
//         otherPos                   = targetPos + Facing::DIRECTION[0];
//         BaseCircuitComponent* down = graph.getComponent(otherPos, CircuitComponentType::PoweredBlockComponent);
//         if (!down || !down->mAllowPowerUp) {
//             for (auto facing = 2; facing <= 5; facing++) {
//                 checkComponent(
//                     graph,
//                     powerAssociationMap,
//                     facing,
//                     otherPos,
//                     info,
//                     positions,
//                     true,
//                     CircuitComponentType::TransporterComponent
//                 );
//             }
//         }
//     } else if (typeID != CircuitComponentType::ProducerComponent || currentComponent->getDirection() != 0) {
//         if (currentComponent->getCircuitComponentType() == CircuitComponentType::BaseRailTransporter) {
//             BlockPos otherPos = targetPos + Facing::DIRECTION[1];
//             for (schar facing = 2; facing <= 5; facing++) {
//                 checkComponent(
//                     graph,
//                     powerAssociationMap,
//                     facing,
//                     otherPos,
//                     info,
//                     positions,
//                     false,
//                     CircuitComponentType::BaseRailTransporter
//                 );
//             }
//             otherPos = targetPos + Facing::DIRECTION[0];
//             for (schar facing = 2; facing <= 5; facing++) {
//                 checkComponent(
//                     graph,
//                     powerAssociationMap,
//                     facing,
//                     otherPos,
//                     info,
//                     positions,
//                     true,
//                     CircuitComponentType::BaseRailTransporter
//                 );
//             }
//         }
//     } else {
//         BlockPos               topPos  = targetPos + Facing::DIRECTION[1];
//         BlockPos               downPos = targetPos + Facing::DIRECTION[0];
//         PoweredBlockComponent* topComp =
//             static_cast<PoweredBlockComponent*>(graph.getComponent(topPos,
//             CircuitComponentType::PoweredBlockComponent)
//             );
//         PoweredBlockComponent* downComp =
//             static_cast<PoweredBlockComponent*>(graph.getComponent(downPos,
//             CircuitComponentType::PoweredBlockComponent)
//             );
//         if ((topComp || downComp) && (!downComp || downComp->mAllowAsPowerSource)
//             && (!topComp || topComp->mAllowAsPowerSource)) {
//             if (!(downComp && downComp->mAllowPowerUp)
//                 && (!topComp || topComp->mAllowPowerUp || topComp->mPromotedToProducer)) {
//                 for (schar facing = 2; facing <= 5; facing++) {
//                     checkComponent(
//                         graph,
//                         powerAssociationMap,
//                         facing,
//                         downPos,
//                         info,
//                         positions,
//                         true,
//                         CircuitComponentType::BaseRailTransporter
//                     );
//                 }
//             }
//         }
//     }
// }

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     FindRelationships_Hook_rewrite,
//     ll::memory::HookPriority::Normal,
//     ::CircuitSceneGraph,
//     &CircuitSceneGraph::findRelationships,
//     void,
//     const ::BlockPos&       pos,
//     ::BaseCircuitComponent* producerTarget,
//     ::BlockSource*          region
// ) {
//     std::queue<CircuitTrackingInfo> stack;
//     CircuitTrackingInfo             startInfo(producerTarget, pos, 0);

//     auto powerAssociationIter = this->mPowerAssociationMap.find(pos);
//     if (powerAssociationIter == mPowerAssociationMap.end()) {
//         this->mPowerAssociationMap.insert({pos, CircuitComponentList()});
//     }
//     powerAssociationIter = this->mPowerAssociationMap.find(pos);
//     stack.push(startInfo);
//     do {
//         CircuitTrackingInfo fillTrack = stack.front();
//         stack.pop();
//         BlockPos              targetPos{fillTrack.mNearest.mPos};
//         BaseCircuitComponent* currentComponent = fillTrack.mNearest.mComponent;
//         if (currentComponent) {
//             int      dampening = fillTrack.mDampening;
//             FacingID dir       = static_cast<FacingID>(currentComponent->getDirection());
//             if (dampening <= 15) {
//                 if (currentComponent->getCircuitComponentType() != CircuitComponentType::PoweredBlockComponent) {
//                     for (int facing = 0; facing < 6; facing++) {
//                         BlockPos newPos = targetPos + Facing::DIRECTION[facing];
//                         if (!this->getBaseComponent(newPos) && region) {
//                             if (region->hasChunksAt(newPos, 0, false)) {
//                                 PoweredBlockComponent* poweredBlock = nullptr;
//                                 const ::Block&         block        = region->getBlock(newPos);
//                                 if (!block.isAir()) {
//                                     if (block.isSolid() || block.hasProperty(BlockProperty::Power_BlockDown)) {
//                                         TargetBlock_Internal::createPoweredBlockComponentAtPos(*region, newPos);
//                                     }
//                                 }
//                             } else {
//                                 ChunkPos chunkPos{newPos};
//                                 this->mComponentsToReEvaluate[::BlockPos{chunkPos, 0}].emplace_back(targetPos);
//                             }
//                         }
//                     }
//                 }
//                 for (int face = 0; face < 6; face++) {
//                     searchForRelationshipAt(region, powerAssociationIter->second, *this, fillTrack, face, stack);
//                 }
//                 searchForRelationhshipsAboveAndBelow(
//                     *this,
//                     powerAssociationIter->second,
//                     currentComponent,
//                     targetPos,
//                     fillTrack,
//                     dir,
//                     stack
//                 );
//             }
//         }
//     } while (!stack.empty());
// }

} // namespace fh
