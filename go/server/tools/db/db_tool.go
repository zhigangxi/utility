package main

import (
	game "server/game/server"
	login "server/login/server"
	"time"

	_ "github.com/go-sql-driver/mysql"
	"xorm.io/xorm"
)

var engine *xorm.Engine

func main() {
	var err error
	engine, err = xorm.NewEngine("mysql", "root:123456@/login?charset=utf8mb4")
	if err != nil {
		println(err.Error())
		return
	}
	if err = engine.Ping(); err != nil {
		println(err.Error())
		return
	}
	if err = engine.Sync2(&login.User{}); err != nil {
		println(err.Error())
		return
	}

	engine, err = xorm.NewEngine("mysql", "root:123456@/game?charset=utf8mb4")
	if err != nil {
		println(err.Error())
		return
	}
	if err = engine.Ping(); err != nil {
		println(err.Error())
		return
	}
	if err = engine.Sync2(&game.User{}); err != nil {
		println(err.Error())
		return
	}
	// var user login.User
	// user.Name = "hello word2"
	// user.Password = "[]byte{0, 0, 20, 10, 0, 0}"
	// var user1 login.User
	// user1.Name = "xzg23"
	// user1.Password = "[]byte{0, 0, 20, 10, 0, 0}"
	//engine.Query()
	// results, err := engine.QueryInterface("select * from user")
	// //engine.Table("user").Where("user=?", 1)
	// if err == nil {
	// 	for k, v := range results {
	// 		fmt.Println(k)
	// 		fmt.Println(v)
	// 	}
	// }
	// num, err := engine.Insert(&user, &user1)
	// if err != nil {
	// 	fmt.Printf("num is:%d,error is:%s\n", num, err.Error())
	// }
	// fmt.Printf("insert num is:%d\n", num)
	//engine.Table("user").Where("user=?", 1).Find()

	//engine.NewSession().Commit()
	//engine.Where()
	//engine.Sum
	//results, err := engine.Where("a = 1").QueryInterface()
	// if _, err = engine.ID(1).Update(&user); err != nil {
	// 	fmt.Print(err)
	// }
	time.Sleep(time.Second * 10)
}
