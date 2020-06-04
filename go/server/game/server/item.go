package game

import (
	"server/utility"
	"sync/atomic"
	"time"
)

type Item struct {
	ID    int64
	Class *ItemClass
}

type ItemClass struct {
	ID   int32    `json:"id"`
	Type ItemType `json:"type"`
}

type ItemType int16

const (
	ItemTypeClothes ItemType = 1
)

var (
	itemClassMgr map[int32]*ItemClass
	maxItemID    int64
)

func GetItemClass(classID int32) *ItemClass {
	itemClass, ok := itemClassMgr[classID]
	if ok {
		return itemClass
	}
	return nil
}

func CreateItem(classID int32, item *Item) bool {
	itemClass, ok := itemClassMgr[classID]
	if !ok {
		return false
	}
	atomic.AddInt64(&maxItemID, 1)
	item.ID = maxItemID
	item.Class = itemClass
	return true
}

func InitItemClass(configPath string) bool {
	if itemClassMgr != nil {
		return false
	}
	maxItemID = time.Now().UnixNano() / 1e6
	itemClassMgr = make(map[int32]*ItemClass)

	var items []*ItemClass
	if !utility.ReadJSONFile(configPath+"/item_class.json", &items) {
		return false
	}
	for _, v := range items {
		itemClassMgr[v.ID] = v
	}
	return true
}
