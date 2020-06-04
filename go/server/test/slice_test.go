package test

import (
	"fmt"
	"math/rand"
	"path"
	"testing"
	"time"
)

func TestSplit(t *testing.T) {
	var intarr []int
	for i := 0; i < 100; i++ {
		intarr = append(intarr, i)
	}
	var s = intarr[10:]
	fmt.Println(s)
}

func readMap(intMap map[int]int) {
	for {
		fmt.Println(intMap[rand.Intn(0xfffff)])
		// for i := range intMap {
		// 	fmt.Println(i)
		// }
		//time.Sleep(time.Second)
	}
}

func writeMap(intMap map[int]int) {
	if len(intMap) > 0 {
		var pos = rand.Intn(len(intMap))
		var num = 0
		for k, _ := range intMap {
			if pos == num {
				delete(intMap, k)
			}
			num++
		}
	}
	var r = rand.Intn(0xfffffff)
	intMap[r] = r
}
func TestMapReadWrite(t *testing.T) {
	var intMap = make(map[int]int)
	go readMap(intMap)
	for {
		writeMap(intMap)
		fmt.Println(intMap)
	}
}

func TestMapGORead(t *testing.T) {
	var intMap = make(map[int]int)
	for i := 0; i < 1000; i++ {
		intMap[i] = 2 * i
	}
	rand.Seed(time.Now().Unix())
	go func() {
		for {
			var i = intMap[rand.Intn(2000)]
			if i != 0 {
				fmt.Println(i)
			}
		}
	}()
	go func() {
		for {
			var i = intMap[rand.Intn(2000)]
			if i != 0 {
				fmt.Println(i)
			}
		}
	}()
	go func() {
		for {
			var i = intMap[rand.Intn(2000)]
			if i != 0 {
				fmt.Println(i)
			}
		}
	}()
	for i := 0; i < 10; i++ {
		time.Sleep(time.Second)
	}
}

func TestNillSlice(t *testing.T) {
	var intArray []int
	intArray = intArray[:0]
	intArray = append(intArray, 1)
	intArray = append(intArray, 1)
	intArray = append(intArray, 1)
	fmt.Print(intArray)
}

func checkSliceSafety(arr []int) {
	_ = arr[3] // early bounds check to guarantee safety of writes below
	fmt.Println(arr[0])
	fmt.Println(arr[1])
	fmt.Println(arr[2])
	fmt.Println(arr[3])
}

func TestCheck(t *testing.T) {
	arr := []int{0, 1}
	checkSliceSafety(arr)
}

func TestSplitPath(t *testing.T) {
	base := path.Base("E:\\work\\doc\\fish_fish_type.json")
	fmt.Printf("%s\n", base)
}

func TestClearMap(t *testing.T) {
	var num = 10000
	var intMap = make(map[int]int, num)
	var clearTimes = 1000
	var now = time.Now().UnixNano()
	for j := 0; j < clearTimes; j++ {
		for i := 0; i < num; i++ {
			intMap[i] = i
		}
		for i := 0; i < num; i++ {
			delete(intMap, i)
		}
	}
	fmt.Printf("%d\n", (time.Now().UnixNano()-now)/1e6)

	now = time.Now().UnixNano()
	for j := 0; j < clearTimes; j++ {
		for i := 0; i < num; i++ {
			intMap[i] = i
		}
		intMap = make(map[int]int, num)
	}
	fmt.Printf("%d\n", (time.Now().UnixNano()-now)/1e6)
	time.Now().UnixNano()
}
