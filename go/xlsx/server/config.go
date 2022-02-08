// Code generated by config tool. DO NOT EDIT.
package config

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
)

type ActorType int32

const (
	Actor   ActorType = 1
	Pharah  ActorType = 2
	Junkrat ActorType = 3
	Genji   ActorType = 4
	Mercy   ActorType = 5
)

type HeroType int32

const (
	NoneHero    HeroType = 1
	PharahHero  HeroType = 2
	JunkratHero HeroType = 3
	GenjiHero   HeroType = 4
	MercyHero   HeroType = 5
	HelloHero   HeroType = 6
)

type ATableData struct {
	Id     int32
	Name   []string
	Weight string
	Hello  []float32
	World  ActorType
	Ext    interface{}
}

type KvData struct {
	Id    []int32
	Name  string
	Paaaa int32
}

type ConfigData struct {
	ATableDataData     []*ATableData
	ATableDataDataById map[int32]*ATableData `json:"-"`
	KvDataData         *KvData
	postHandlers       []func(*ConfigData) `json:"-"`
}

var (
	cfgData ConfigData
)

func LoadConfigData(file string) bool {
	data, err := ioutil.ReadFile(file)
	if err != nil {
		fmt.Println(err.Error())
		return false
	}
	err = json.Unmarshal(data, &cfgData)
	if err != nil {
		fmt.Println(err.Error())
		return false
	}
	cfgData.ATableDataDataById = make(map[int32]*ATableData)
	for _, v := range cfgData.ATableDataData {
		if cfgData.ATableDataDataById[v.Id] != nil {
			return false
		}
		cfgData.ATableDataDataById[v.Id] = v
	}

	for _, v := range cfgData.postHandlers {
		v(&cfgData)
	}
	return true
}

func RegisterLoadHandler(f func(*ConfigData)) {
	cfgData.postHandlers = append(cfgData.postHandlers, f)
}

func GetATableDataById(key int32) *ATableData {
	return cfgData.ATableDataDataById[key]
}

func GetATableData() []*ATableData {
	return cfgData.ATableDataData
}

func GetKvData() *KvData {
	return cfgData.KvDataData
}
