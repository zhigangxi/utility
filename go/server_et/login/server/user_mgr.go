package login

import (
	"server/network"
	"sync"
	"time"
)

const (
	maxUserNum    = 2000
	maxSessionNum = 20000
)

type userMgr struct {
	sessLock sync.RWMutex
	userLock sync.RWMutex
	users    map[int64]User
	sessions map[int64]userSession
}

func newUserMgr() *userMgr {
	var mgr = new(userMgr)
	mgr.users = make(map[int64]User, maxUserNum)
	mgr.sessions = make(map[int64]userSession, maxSessionNum)
	return mgr
}

func (mgr *userMgr) addUser(user User, session *network.Session) {
	session.SetUser(user)
	mgr.userLock.Lock()
	user.Session = session
	mgr.users[user.ID] = user
	mgr.userLock.Unlock()
}

func (mgr *userMgr) getUser(userID int64) User {
	var user User
	mgr.userLock.RLock()
	user = mgr.users[userID]
	mgr.userLock.RUnlock()
	return user
}

func (mgr *userMgr) delUser(userID int64) {
	mgr.userLock.Lock()
	delete(mgr.users, userID)
	mgr.userLock.Unlock()
}

func (mgr *userMgr) addUserSession(userID int64, session string) {
	var userSess = userSession{session: session, addTime: time.Now().Unix()}
	mgr.sessLock.Lock()
	mgr.sessions[userID] = userSess
	mgr.sessLock.Unlock()
}

func (mgr *userMgr) getUserSession(userID int64) string {
	var useSess userSession
	mgr.sessLock.RLock()
	useSess = mgr.sessions[userID]
	mgr.sessLock.RUnlock()
	return useSess.session
}

func (mgr *userMgr) delUserSession(userID int64) {
	mgr.sessLock.Lock()
	delete(mgr.sessions, userID)
	mgr.sessLock.Unlock()
}
