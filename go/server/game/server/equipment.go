package game

type Equipment struct {
	Item Item
	Pos  uint32
}

type UserEquipments []*Equipment

func NewUserEquipments(maxNum int) UserEquipments {
	return make([]*Equipment, maxNum)
}

func (es *UserEquipments) GetByPos(pos uint32) *Equipment {
	if pos >= uint32(len(*es)) {
		return nil
	}
	return (*es)[pos]
}

func (es *UserEquipments) GetByID(id int64) *Equipment {
	for _, v := range *es {
		if v.Item.ID == id {
			return v
		}
	}
	return nil
}

func (es *UserEquipments) SetPackage(equip *Equipment, classID int32) bool {
	if equip.Pos >= uint32(len(*es)) {
		return false
	}
	equip.Item.Class = GetItemClass(classID)
	if equip.Item.Class != nil {
		return false
	}
	(*es)[equip.Pos] = equip
	return true
}

func (es *UserEquipments) AddEquip(item *Item, pos uint32) bool {
	if pos >= uint32(len(*es)) {
		return false
	}
	if (*es)[pos] != nil {
		return false
	}
	equip := new(Equipment)
	equip.Item = *item
	equip.Pos = pos
	(*es)[pos] = equip
	return true
}

func (es *UserEquipments) DelEquip(pos uint32) bool {
	var equip = es.GetByPos(pos)
	if equip == nil {
		return false
	}
	(*es)[pos] = nil
	return true
}
