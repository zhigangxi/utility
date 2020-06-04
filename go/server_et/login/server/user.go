package login

import (
	"server/network"
	"time"
)

type User struct {
	ID        int64
	Name      string           `xorm:"varchar(32) not null unique"`
	Password  string           `xorm:"varchar(32) not null"`
	CreatedAt time.Time        `xorm:"created"`
	Session   *network.Session `xorm:"-"` //-	这个Field将不进行字段映射

}

type userSession struct {
	addTime int64
	session string
}
