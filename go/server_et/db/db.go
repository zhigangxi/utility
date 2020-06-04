package database

import (
	"log"
	"runtime/debug"

	_ "github.com/go-sql-driver/mysql"
	"xorm.io/xorm"
)

type Database struct {
	engine         *xorm.Engine
	dataSourceName string
	querys         chan Query
	results        chan Query
	goroutineNum   int
}

func NewDB(dataSourceName string) (*Database, error) {
	engine, err := xorm.NewEngine("mysql", dataSourceName)
	if err != nil {
		return nil, err
	}
	if err = engine.Ping(); err != nil {
		engine.Close()
		return nil, err
	}
	db := new(Database)
	db.engine = engine
	db.dataSourceName = dataSourceName
	return db, nil
}

type Query interface {
	QueryCB(engine *xorm.Engine)
	ResultCB()
}

// asynchronous func
//配合要使用RunResult和Query
func (db *Database) Start(goroutineNum int) {
	if goroutineNum <= 0 {
		log.Panicf("goroutineNum:%d error", goroutineNum)
		return
	}
	if db.goroutineNum != 0 {
		log.Printf("DB goroutine is already start!")
		return
	}
	db.querys = make(chan Query, goroutineNum*4)
	db.results = make(chan Query, goroutineNum*2)
	db.goroutineNum = goroutineNum
	for i := 0; i < db.goroutineNum; i++ {
		go db.runQuery()
	}
}

func (db *Database) runQuery() {
	defer func() {
		x := recover()
		if x != nil {
			log.Printf("%v\n%s", x, string(debug.Stack()))
		}
	}()

	for query := range db.querys {
		query.QueryCB(db.engine)
		db.results <- query
	}
}

func (db *Database) GetResult() *chan Query {
	return &db.results
}

// func (db *Database) RunResult() bool {
// 	select {
// 	case result := <-db.results:
// 		(*result).ResultCB()
// 		return true
// 	default:
// 		return false
// 	}
// }

func (db *Database) Query(query Query) {
	db.querys <- query
}

// end asynchronous func

//synchronize func
func (db *Database) GetEngine() *xorm.Engine {
	return db.engine
}

func (db *Database) Insert(beans ...interface{}) bool {
	session := db.engine.NewSession()
	defer session.Close()
	if err := session.Begin(); err != nil {
		log.Println(err)
		return false
	}
	if _, err := db.engine.Insert(beans...); err != nil {
		log.Println(err)
		return false
	}
	if err := session.Commit(); err != nil {
		log.Println(err)
		return false
	}
	return true
}

func (db *Database) InsertOne(bean interface{}) bool {
	if _, err := db.engine.InsertOne(bean); err != nil {
		return true
	} else {
		log.Println(err)
		return false
	}
}

func (db *Database) Get(session *xorm.Session, bean interface{}) bool {
	if session != nil {
		if _, err := session.Get(bean); err != nil {
			log.Println(err)
			return false
		}
	} else {
		if _, err := db.engine.Get(bean); err != nil {
			log.Println(err)
			return false
		}
	}
	return true
}

//beans could be []Struct, []*Struct, map[int64]Struct map[int64]*Struct
func (db *Database) Find(session *xorm.Session, rowsSlicePtr interface{}) bool {
	if err := session.Find(rowsSlicePtr); err != nil {
		log.Println(err)
		return false
	}
	return true
}

func (db *Database) Update(session *xorm.Session, bean interface{}) bool {
	if _, err := session.Update(bean); err != nil {
		log.Println(err)
		return true
	}
	return false
}

func (db *Database) Delete(session *xorm.Session, bean interface{}) bool {
	if _, err := session.Delete(bean); err != nil {
		log.Println(err)
		return true
	}
	return false
}

func (db *Database) Count(session *xorm.Session, bean interface{}) int64 {
	var num int64
	var err error
	if session == nil {
		if num, err = db.engine.Count(bean); err != nil {
			log.Println(err)
			return 0
		}
	} else {
		if num, err = session.Count(bean); err != nil {
			log.Println(err)
			return 0
		}
	}
	return num
}

func (db *Database) Sum(session *xorm.Session, bean interface{}, colName string) float64 {
	var num float64
	var err error
	if session == nil {
		if num, err = db.engine.Sum(bean, colName); err != nil {
			log.Println(err)
			return 0
		}
	} else {
		if num, err = session.Sum(bean, colName); err != nil {
			log.Println(err)
			return 0
		}
	}
	return num
}

func (db *Database) SumInt(session *xorm.Session, bean interface{}, colName string) int64 {
	var num int64
	var err error
	if session == nil {
		if num, err = db.engine.SumInt(bean, colName); err != nil {
			log.Println(err)
			return 0
		}
	} else {
		if num, err = session.SumInt(bean, colName); err != nil {
			log.Println(err)
			return 0
		}
	}
	return num
}

func (db *Database) Sums(session *xorm.Session, bean interface{}, colNames ...string) []float64 {
	var num []float64
	var err error
	if session == nil {
		if num, err = db.engine.Sums(bean, colNames...); err != nil {
			log.Println(err)
			return nil
		}
	} else {
		if num, err = session.Sums(bean, colNames...); err != nil {
			log.Println(err)
			return nil
		}
	}
	return num
}

func (db *Database) SumsInt(session *xorm.Session, bean interface{}, colNames ...string) []int64 {
	var num []int64
	var err error
	if session == nil {
		if num, err = db.engine.SumsInt(bean, colNames...); err != nil {
			log.Println(err)
			return nil
		}
	} else {
		if num, err = session.SumsInt(bean, colNames...); err != nil {
			log.Println(err)
			return nil
		}
	}
	return num
}

//end synchronize func
