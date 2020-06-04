package game

import (
	"log"
	database "server/db"
	"server/network"
	gamepb "server/proto/game"
	"time"

	"xorm.io/xorm"
)

const (
	AdminLvCmdMe    = 1 //管理员等级，可操作自己
	AdminLvCmdOther = 2 //可对其他玩家操作
	MaxUserPackNum  = 64
	MaxUserEquipNum = 16
)

type User struct {
	ID         int64
	Nick       string `xorm:"varchar(32) not null unique"`
	Level      int32
	Exp        int32
	Score      int64 //游戏币
	Gold       int32 //充值币
	AdminLevel int16 //管理员等级，0为普通用户
	Package    []byte
	Equipment  []byte
	Session    *network.Session `xorm:"-"` //-	这个Field将不进行字段映射
	packages   UserPackage      `xorm:"-"`
	equipments UserEquipments   `xorm:"-"`
	CreatedAt  time.Time        `xorm:"created"`
	UpdatedAt  time.Time        `xorm:"updated"`
}

func (user *User) Init() {
	if user.packages == nil {
		user.packages = NewUserPackages(MaxUserPackNum)
	}
	if user.equipments == nil {
		user.equipments = NewUserEquipments(MaxUserEquipNum)
	}
	if len(user.Package) > 0 {
		var pbPack gamepb.PackageAck
		unPbPanic(user.Package, &pbPack)
		for _, v := range pbPack.Infos {
			var pack Package
			pack.Item.ID = v.Id
			pack.Num = v.Num
			pack.Pos = v.Pos
			if !user.packages.SetPackage(&pack, v.ClassID) {
				log.Printf("Set user %d package itemID:%d error\n", user.ID, v.ClassID)
			}
		}
	}
	if len(user.Equipment) > 0 {
		var pbEquip gamepb.EquipmentAck
		unPbPanic(user.Equipment, &pbEquip)
		for _, v := range pbEquip.Equips {
			var equip Equipment
			equip.Pos = v.Pos
			if !user.equipments.SetPackage(&equip, v.ClassID) {
				log.Printf("Set user %d package equip:%d error\n", user.ID, v.ClassID)
			}
		}
	}
}

type saveUser struct {
	user *User
	err  error
}

func (su *saveUser) QueryCB(engine *xorm.Engine) {
	_, su.err = engine.ID(su.user.ID).Update(su.user)
}

func (su *saveUser) ResultCB() {
	if su.err != nil {
		log.Printf("update user:%v error", su.user)
	}
}

func (user *User) Save(db *database.Database) {
	var su saveUser
	var packPb gamepb.PackageAck
	var equipPb gamepb.EquipmentAck
	copyUserPackToPb(&user.packages, &packPb)
	copyUserEquipToPb(&user.equipments, &equipPb)
	user.Package = marshalPbPanic(&packPb)
	user.Equipment = marshalPbPanic(&equipPb)
	db.Query(&su)
}
