#include "MinecraftLoadAndCheck.h"
#include "Application.h"
#include "MinecraftInstance.h"
#include "PackProfile.h"

MinecraftLoadAndCheck::MinecraftLoadAndCheck(MinecraftInstance* inst, Net::Mode netmode) : m_inst(inst), m_netmode(netmode) {}

void MinecraftLoadAndCheck::executeTask()
{
    // add offline metadata load task
    auto* components = m_inst->getPackProfile();
    if (m_netmode == Net::Mode::Online && m_inst->settings()->get("UseLatestMinecraftVersion").toBool()) {
        auto releaseType = m_inst->settings()->get("UseLatestMinecraftVersionType").toString();
        if (components->updateLatestMinecraft(releaseType == "release") && APPLICATION->settings()->get("AutomaticJavaSwitch").toBool() &&
            m_inst->settings()->get("AutomaticJava").toBool() && m_inst->settings()->get("OverrideJavaLocation").toBool()) {
            m_inst->settings()->set("OverrideJavaLocation", false);
            m_inst->settings()->set("JavaPath", "");
        }
    }
    if (auto result = components->reload(m_netmode); !result) {
        emitFailed(result.error());
        return;
    }
    m_task = components->getCurrentTask();

    if (!m_task) {
        checkLaunchProfile();
        return;
    }
    connect(m_task.get(), &Task::succeeded, this, &MinecraftLoadAndCheck::checkLaunchProfile);
    connect(m_task.get(), &Task::failed, this, &MinecraftLoadAndCheck::emitFailed);
    connect(m_task.get(), &Task::aborted, this, &MinecraftLoadAndCheck::emitAborted);
    propagateFromOther(m_task.get());
}

void MinecraftLoadAndCheck::checkLaunchProfile()
{
    const auto profile = m_inst->getPackProfile()->getProfile();
    if (const auto result = validateLaunchProfile(*profile); !result) {
        emitFailed(result.error());
        return;
    }
    emitSucceeded();
}

Result<> MinecraftLoadAndCheck::validateLaunchProfile(const LaunchProfile& profile)
{
    if (profile.getProblemSeverity() == ProblemSeverity::Error || profile.getMainClass().isEmpty() || !profile.getMainJar()) {
        return std::unexpected(tr("Minecraft components are incomplete or contain errors. Reload the version components before launching."));
    }
    return {};
}

bool MinecraftLoadAndCheck::canAbort() const
{
    if (m_task) {
        return m_task->canAbort();
    }
    return true;
}

bool MinecraftLoadAndCheck::abort()
{
    if (m_task && m_task->canAbort()) {
        return m_task->abort();
    }
    return Task::abort();
}
