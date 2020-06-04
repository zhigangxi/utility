package game

import (
	"log"
	gamepb "server/proto/game"
	"server/utility"

	"github.com/golang/protobuf/proto"
)

type GameScene struct {
	SceneID    int32
	BornMinPos Point
	BornMaxPos Point
	JumpPoint  []JumpPointInfo
	Users      map[int64]*User
}

type JumpPointInfo struct {
	FromID  int32 //from game scene id
	FromMin Point
	FromMax Point
	ToID    int32 //to game scene id
	ToMin   Point
	ToMax   Point
}

type Point struct {
	x int32
	y int32
}

type sceneCfg struct {
	ID     int32   `json:"id"`
	MinPos []int32 `json:"min_pos"`
	MaxPos []int32 `json:"max_pos"`
}

type jumpPointCfg struct {
	ID      int32   `json:"id"`
	FromID  int32   `json:"from_id"`
	FromMin []int32 `json:"from_min"`
	FromMax []int32 `json:"from_max"`
	ToID    int32   `json:"to_id"`
	ToMin   []int32 `json:"to_min"`
	ToMax   []int32 `json:"to_max"`
}

func (jump *JumpPointInfo) InJumpPoint(x int32, y int32) bool {
	return x >= jump.FromMin.x && x <= jump.FromMax.x && y <= jump.FromMax.y && y >= jump.FromMin.y
}

func (server *Server) initGameScene() bool {
	if server.gameScenes != nil {
		log.Printf("scene is already init\n")
		return false
	}
	server.gameScenes = make(map[int32]*GameScene)

	var path = server.config.ConfigPath
	var sceneCfgs []sceneCfg
	if !utility.ReadJSONFile(path+"/scene_info.json", &sceneCfgs) {
		log.Printf("read scene_info.json error\n")
		return false
	}

	var jumpPointCfgs []jumpPointCfg
	if !utility.ReadJSONFile(path+"/scene_jump.json", &jumpPointCfgs) {
		log.Printf("read scene_jump.json error\n")
		return false
	}
	for _, v := range sceneCfgs {
		if len(v.MaxPos) != 2 {
			log.Printf("scene:%d max pox error\n", v.ID)
			return false
		}
		if len(v.MinPos) != 2 {
			log.Printf("scene:%d min pox error\n", v.ID)
			return false
		}
		var scene = new(GameScene)
		scene.SceneID = v.ID
		scene.BornMaxPos.x = utility.MaxInt32(v.MaxPos[0], v.MinPos[0])
		scene.BornMaxPos.y = utility.MaxInt32(v.MaxPos[1], v.MinPos[1])
		scene.BornMinPos.x = utility.MinInt32(v.MaxPos[0], v.MinPos[0])
		scene.BornMinPos.y = utility.MinInt32(v.MaxPos[1], v.MinPos[1])
		server.gameScenes[v.ID] = scene
	}
	for _, v := range jumpPointCfgs {
		fromScene := server.gameScenes[v.FromID]
		toScene := server.gameScenes[v.ToID]
		if fromScene == nil || toScene == nil {
			log.Printf("jump point from(%d) or to scene(%d) id error\n", v.FromID, v.ToID)
			return false
		}
		if len(v.FromMax) != 2 || len(v.FromMin) != 2 || len(v.ToMax) != 2 || len(v.ToMin) != 2 {
			log.Printf("jump point:%d from or to point error\n", v.ID)
			return false
		}
		var jump JumpPointInfo
		jump.FromID = v.FromID
		jump.ToID = v.ToID
		jump.FromMax.x = utility.MaxInt32(v.FromMax[0], v.FromMin[0])
		jump.FromMax.y = utility.MaxInt32(v.FromMax[1], v.FromMin[1])
		jump.FromMin.x = utility.MinInt32(v.FromMax[0], v.FromMin[0])
		jump.FromMin.y = utility.MinInt32(v.FromMax[1], v.FromMin[1])
		jump.ToMax.x = utility.MaxInt32(v.ToMax[0], v.ToMax[0])
		jump.ToMax.y = utility.MaxInt32(v.ToMax[1], v.ToMax[1])
		jump.ToMax.x = utility.MinInt32(v.ToMin[0], v.ToMin[0])
		jump.ToMax.y = utility.MinInt32(v.ToMin[1], v.ToMin[1])
		fromScene.JumpPoint = append(fromScene.JumpPoint, jump)
	}
	return true
}

func randomPoint(min Point, max Point) Point {
	var p Point
	p.x = utility.RandomInt32(min.x, max.x)
	p.y = utility.RandomInt32(min.y, max.y)
	return p
}

func (scene *GameScene) Enter(user *User) {
	if user.scene != nil {
		user.scene.Exit(user)
	}
	user.scene = scene
	if user.SceneID != scene.SceneID {
		var p = randomPoint(scene.BornMinPos, scene.BornMaxPos)
		user.X = p.x
		user.Y = p.y
	}
	user.SceneID = scene.SceneID
	scene.Users[user.ID] = user
	var sendMsg gamepb.EnterScene
	sendMsg.SceneID = scene.SceneID
	sendMsg.X = user.X
	sendMsg.Y = user.Y
	scene.BrocastMsg(&sendMsg)
}

func (scene *GameScene) Exit(user *User) {
	var sendMsg gamepb.PlayerExitScene
	sendMsg.UserID = user.ID
	scene.BrocastMsg(&sendMsg)
	delete(scene.Users, user.ID)
}

func (scene *GameScene) BrocastMsg(pb proto.Message) {
	var msg = pbToByte(pb)
	if msg == nil {
		return
	}
	for _, v := range scene.Users {
		v.Session.SendMsg(msg)
	}
}

func (server *Server) EnterScene(user *User, sceneID int32) bool {
	var scene = server.gameScenes[sceneID]
	if scene == nil {
		return false
	}
	scene.Enter(user)
	return true
}
