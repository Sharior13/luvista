#include "userSession.h"
#include "appPaths.h"

UserSession::UserSession()
{
    AppPaths::setActiveProfile(Profiles::GUEST);       // before anyone logs in everything goes into the guest folder
}


std::vector<std::string> UserSession::cards() const
{
    return profiles.shown();
}


void UserSession::enter(const std::string& profile)
{
    active = profile;
    guest = false;
    AppPaths::setActiveProfile(profile);
}


bool UserSession::select(const std::string& name)
{
    std::string actual = profiles.find(name);

    if (actual == "" || profiles.needsPassword(actual))
        return false;                                  // a locked card is opened with signIn(), never by a click alone

    enter(actual);
    return true;
}


bool UserSession::isLocked(const std::string& name) const
{
    return profiles.needsPassword(name);
}


bool UserSession::signIn(const std::string& userOrEmail, const std::string& password, std::string& error)
{
    std::string who = profiles.signIn(userOrEmail, password, error);

    if (who == "")
        return false;

    enter(who);
    return true;
}


void UserSession::startGuest()
{
    profiles.resetGuest();                             // nothing of a former guest is left
    active = "Guest";
    guest = true;
    AppPaths::setActiveProfile(Profiles::GUEST);
}


bool UserSession::create(const std::string& name, const std::string& email, const std::string& password,
                         bool remember, std::string& error)
{
    if (!profiles.createAccount(name, email, password, remember, error))
        return false;

    pending = profiles.find(name);
    return true;
}


bool UserSession::setPendingPicture(const std::string& imagePath)
{
    return !pending.empty() && profiles.setPicture(pending, imagePath);
}


std::string UserSession::pendingPicture() const
{
    return pending.empty() ? "" : profiles.picture(pending);
}


bool UserSession::finishCreate()
{
    if (pending.empty())
        return false;

    enter(pending);
    pending = "";
    return true;
}


void UserSession::signOut()
{
    active = "";
    guest = false;
    pending = "";
    AppPaths::setActiveProfile(Profiles::GUEST);
}


bool UserSession::isLoggedIn() const { return !active.empty(); }
bool UserSession::isGuest() const { return guest; }
const std::string& UserSession::name() const { return active; }


std::string UserSession::initial() const
{
    return Profiles::initial(active);
}


std::string UserSession::email() const
{
    return guest ? "" : profiles.email(active);
}


std::string UserSession::picture() const
{
    return guest ? "" : profiles.picture(active);
}


bool UserSession::setPicture(const std::string& imagePath)
{
    return isLoggedIn() && !guest && profiles.setPicture(active, imagePath);
}


bool UserSession::updateAccount(const std::string& newName, const std::string& email,
                                const std::string& currentPassword, const std::string& newPassword, std::string& error)
{
    if (!isLoggedIn() || guest) {
        error = "Guest mode has no account to edit";
        return false;
    }

    std::string name = active;

    if (!profiles.updateAccount(name, newName, email, currentPassword, newPassword, error))
        return false;

    enter(name);                                       // the folder may have a new name
    return true;
}


std::string UserSession::pictureOf(const std::string& profile) const
{
    return profiles.picture(profile);
}


std::string UserSession::initialOf(const std::string& profile) const
{
    return Profiles::initial(profile);
}
