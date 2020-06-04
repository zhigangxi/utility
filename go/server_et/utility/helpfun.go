package utility

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"math/rand"
)

//读取jison文件
func ReadJSONFile(fileName string, jsFile interface{}) bool {
	var data, err = ioutil.ReadFile(fileName)
	if err != nil {
		log.Printf("can not open %s file", fileName)
		return false
	}
	var e = json.Unmarshal(data, jsFile)
	if e != nil {
		log.Print(e)
		return false
	}
	return true
}

func Random(min int, max int) int {
	if min >= max {
		log.Panicf("randdom min(%d) >= max(%d)", min, max)
		return 0
	}
	return rand.Intn(max-min) + min
}

func RandomInt32(min int32, max int32) int32 {
	if min >= max {
		log.Panicf("randdom min(%d) >= max(%d)", min, max)
		return 0
	}
	return int32(rand.Intn(int(max-min)) + int(min))
}

func MaxInt32(num1 int32, num2 int32) int32 {
	if num1 > num2 {
		return num1
	}
	return num2
}

func MinInt32(num1 int32, num2 int32) int32 {
	if num1 > num2 {
		return num2
	}
	return num1
}
