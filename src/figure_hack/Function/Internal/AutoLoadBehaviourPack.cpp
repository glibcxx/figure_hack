#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>

#include <mc/deps/core/resource/PackIdVersion.h>
#include <mc/deps/core/sem_ver/SemVersion.h>
#include <mc/deps/core/string/StaticOptimizedString.h>
#include <mc/deps/core/utility/NonOwnerPointer.h>
#include <mc/platform/UUID.h>
#include <mc/resources/PackInstance.h>
#include <mc/resources/PackSettingsFactory.h>
#include <mc/resources/ResourcePack.h>
#include <mc/resources/ResourcePackManager.h>
#include <mc/resources/ResourcePackRepository.h>
#include <mc/resources/ResourcePackStack.h>


namespace fh {

static bool isPackLoaded = false;

LL_AUTO_TYPE_INSTANCE_HOOK(
    T,
    ll::memory::HookPriority::Normal,
    ResourcePackManager,
    &ResourcePackManager::setStack,
    bool,
    ::std::unique_ptr<::ResourcePackStack> stack,
    ::ResourcePackStackType                stackType,
    bool                                   composeStack
) {
    if (stackType == ResourcePackStackType::Addon && !isPackLoaded) {
        PackIdVersion packId = PackIdVersion::fromString("53d6b8ca-ec81-4965-adc9-5d8ee6632f72_1.0.1");
        packId.mPackType     = PackType::Addon;
        optional_ref  repo   = ll::service::getResourcePackRepository();
        ResourcePack* pack   = repo->getResourcePackForPackId(packId);
        stack->add(
            {
                Bedrock::NonOwnerPointer{pack->mControlBlock, pack},
                "",
                false,
                repo->getPackSettingsFactory().getPackSettings(pack->getManifest())
        },
            Bedrock::NonOwnerPointer<const IResourcePackRepository>{repo->mControlBlock, &repo.value()},
            false
        );
        isPackLoaded = true;
    }
    return this->origin(std::move(stack), stackType, composeStack);
}

} // namespace fh