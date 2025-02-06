#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>

#include <mc/deps/core/resource/PackIdVersion.h>
#include <mc/deps/core/sem_ver/SemVersion.h>
#include <mc/deps/core/utility/NonOwnerPointer.h>
#include <mc/platform/UUID.h>
#include <mc/resources/PackInstance.h>
#include <mc/resources/PackSettingsFactory.h>
#include <mc/resources/ResourcePack.h>
#include <mc/resources/ResourcePackManager.h>
#include <mc/resources/ResourcePackRepository.h>
#include <mc/resources/ResourcePackStack.h>

namespace fh {

LL_AUTO_TYPE_INSTANCE_HOOK(
    T,
    ll::memory::HookPriority::Normal,
    ResourcePackManager,
    &ResourcePackManager::handlePendingStackChanges,
    void
) {
    optional_ref             repo  = ll::service::getResourcePackRepository();
    const ResourcePackStack& stack = this->getStack(ResourcePackStackType::Addon);

    PackIdVersion packId{
        mce::UUID::fromString("53d6b8ca-ec81-4965-adc9-5d8ee6632f72"),
        {1, 0, 1, "", ""},
        PackType::Addon
    };
    ResourcePack* pack = repo->getResourcePackSatisfiesPackId(packId, true);
    repo->getResourcePackSatisfiesPackId(packId, true);

    const_cast<ResourcePackStack&>(stack).add(
        {
            Bedrock::NonOwnerPointer{pack->mControlBlock, pack},
            "",
            false,
            repo->getPackSettingsFactory().getPackSettings(pack->getManifest())
    },
        Bedrock::NonOwnerPointer<const IResourcePackRepository>{repo->mControlBlock, &repo.value()},
        false
    );

    this->origin();
}

} // namespace fh