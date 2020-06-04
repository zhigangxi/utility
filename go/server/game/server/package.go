package game

type Package struct {
	Item Item
	Pos  uint32
	Num  int32
}

type UserPackage []*Package

func NewUserPackages(maxNum int) UserPackage {
	return make(UserPackage, maxNum)
}

func (up *UserPackage) GetByPos(pos uint32) *Package {
	if pos >= uint32(len(*up)) {
		return nil
	}
	return (*up)[pos]
}

func (up *UserPackage) GetByID(id int64) *Package {
	for _, v := range *up {
		if v.Item.ID == id {
			return v
		}
	}
	return nil
}

func (up *UserPackage) SetPackage(pack *Package, classID int32) bool {
	if pack.Pos >= uint32(len(*up)) {
		return false
	}
	pack.Item.Class = GetItemClass(classID)
	if pack.Item.Class != nil {
		return false
	}
	(*up)[pack.Pos] = pack
	return true
}

func (up *UserPackage) AddPackage(id int32, num int, pos uint32) bool {
	if pos >= uint32(len(*up)) {
		return false
	}
	if (*up)[pos] == nil {
		return false
	}
	equip := new(Package)
	if !CreateItem(id, &equip.Item) {
		return false
	}
	equip.Pos = pos
	(*up)[pos] = equip
	return true
}

func (up *UserPackage) DelEquip(pos uint32) bool {
	var equip = up.GetByPos(pos)
	if equip == nil {
		return false
	}
	(*up)[pos] = nil
	return true
}
