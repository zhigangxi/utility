package test

import (
	"fmt"
	"server/utility"
	"testing"
	"time"
	"unicode/utf8"

	"github.com/google/uuid"
)

type funData struct {
	i int
}

func (d *funData) dataFun() {
	fmt.Printf("data is:%d\n", d.i)
}

func usingCallback(fun func()) {
	fun()
}

func TestCallback(t *testing.T) {
	var d funData
	d.i = 100
	fmt.Printf("usingCallback\n")
	usingCallback(d.dataFun)
}

func StartRead(b chan []byte, num int) {
	for s := range b {
		fmt.Printf("%s--%d\n", string(s), num)
	}
}

func TestChanRange(tt *testing.T) {
	var b chan []byte
	b = make(chan []byte, 200)
	//StartRead(&b, 1)
	for i := 0; i < 5; i++ {
		go StartRead(b, i+1)
	}
	for i := 0; i < 200; i++ {
		b <- []byte(fmt.Sprintf("hello_%d", i))
		fmt.Println(cap(b))
		time.Sleep(time.Second)
	}
	//time.Sleep(time.Second * 100)
	// b <- []byte("")
	// b <- nil
	// var l = len(b)
	// fmt.Print(l)
	// //var t = time.Now().UnixNano()
	// close(b)
	// for i := 0; i < 105; i++ {
	// 	//t = time.Now().UnixNano()
	// 	select {
	// 	case msg := <-b:
	// 		fmt.Println(msg)
	// 	default:
	// 		fmt.Printf("default\n")
	// 		return
	// 		//t = time.Now().UnixNano()
	// 	}
	// }
	// for msg := range b {
	// 	fmt.Println(msg)
	// }
}

func TestConvertTo(t *testing.T) {
	var i interface{}
	var ii int = 10
	i = &ii
	var j = i.(*float32)
	fmt.Println(j)
}

func TestUUID(t *testing.T) {
	var uid = uuid.Must(uuid.NewRandom())
	fmt.Println(uid)
}

func GetPoint(intPArr []*int, pos int) *int {
	return intPArr[pos]
}

func TestSetNil(t *testing.T) {
	var intPArr []*int
	for i := 0; i < 10; i++ {
		intPArr = append(intPArr, &i)
	}
	var intp = GetPoint(intPArr, 4)
	intp = nil
	fmt.Println(intp)
	fmt.Println(intPArr)
}

type ItemClass struct {
	ID   int32    `json:"id"`
	Type ItemType `json:"type"`
}

type ItemType int16

func TestReadJson(t *testing.T) {
	var items []*ItemClass
	//var items = make(map[int32]*ItemClass)
	if !utility.ReadJSONFile("../bin/server/item_class.json", &items) {
		return
	}
	fmt.Println(items)
}

func TestUtf8Len(t *testing.T) {
	var now = time.Now().UnixNano()
	var num = 0xffffff
	var strLen = 0
	for i := 0; i < num; i++ {
		strLen = utf8.RuneCountInString("hello 世界")
	}
	fmt.Printf("%d:%d\n", strLen, time.Now().UnixNano()-now)

	now = time.Now().UnixNano()
	for i := 0; i < num; i++ {
		strLen = len([]rune("hello 世界"))
	}
	fmt.Printf("%d:%d\n", strLen, time.Now().UnixNano()-now)
}
