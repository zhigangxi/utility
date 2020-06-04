package game

type UserMgr map[int64]*User

func NewUserMgr(maxNum int) *UserMgr {
	var mgr = make(map[int64]*User, maxNum)
	return (*UserMgr)(&mgr)
}

func (mgr *UserMgr) AddUser(user *User) {
	(*mgr)[user.ID] = user
}

func (mgr *UserMgr) DelUser(userID int64) {
	delete(*mgr, userID)
}

func (mgr *UserMgr) GetUser(userID int64) *User {
	user, ok := (*mgr)[userID]
	if ok {
		return user
	}
	return nil
}
