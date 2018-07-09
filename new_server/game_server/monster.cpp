#include "stdafx.h"
#include "monster.h"
#include "xml_data_loader.hpp"
#include "protocol.h"
#include "game_socket.h"
#include "game_player.h"
#include "server_data_base.h"
#include "game_map.h"
#include "global_manager.h"

monster::monster(void)
{
	srand((unsigned)time(NULL)); 
	dead_ = false;
	m_pTarget = NULL;
	m_pMonsterT = NULL;
	m_state = WANDER;
	m_bBeAttack = false;
	m_pAttacker = NULL;
	m_lasttime = GetTickCount();
	m_nModelSize = 100;
	m_dwHitIntervalTime = 1000 MSEC;
	m_lastHitTime = 0;
}

monster::~monster(void)
{
}


void monster::fill_sendclient_pack(CNetMessage &msg)
{
	msg<<insid<<m_nMonsterID<<pos.x_<<pos.y_<<pos.z_<<face_
		<<m_szMonsterTitle<<m_szMonsterName<<m_szMonsterModel<<m_nMonsterLev<<m_nMonsterHP
		<<m_nMonsterMP<<m_szMonsterIcon<<m_fMonsterZoom<<m_eBossTag;
}

void monster::fill_serverdata_pack(CNetMessage& msg)
{
	msg<<insid<<m_nMonsterID<<pos.x_<<pos.y_<<pos.z_<<face_;
}
void monster::fill_data(CNetMessage &msg)
{
	msg>>insid>>m_nMonsterID>>pos.x_>>pos.y_>>pos.z_>>face_;
	originPos  = pos;
	originFace = face_;
	m_pMonsterT	= &(server_data::instance().MonsterTypeData_map[m_nMonsterID]);
	if( m_pMonsterT )
	{
		m_szMonsterTitle 	= 	m_pMonsterT->m_szMonsterTitle;
		m_szMonsterName		= 	m_pMonsterT->m_szMonsterName;
		m_szMonsterModel 	= 	m_pMonsterT->m_szMonsterModel;
		m_nMonsterLev 		= 	m_pMonsterT->m_nMonsterLev;
		m_nMonsterHP 		= 	m_pMonsterT->m_nMonsterHP;
		m_nMonsterMP 		= 	m_pMonsterT->m_nMonsterMP;
		m_szMonsterIcon 	= 	m_pMonsterT->m_szMonsterIcon;
		m_fMonsterZoom 		= 	m_pMonsterT->m_fMonsterZoom;
		m_eBossTag 			= 	m_pMonsterT->m_eBossTag;
		m_nEXP 				= 	m_pMonsterT->m_nEXP;
		m_nSPPro 			= 	m_pMonsterT->m_nSPPro;
		m_nMoneyPro 		= 	m_pMonsterT->m_nMoneyPro;
		m_nMoney 			= 	m_pMonsterT->m_nMoney;
		m_nEquipPro1 		= 	m_pMonsterT->m_nEquipPro1;
		m_nEquipItem1 		= 	m_pMonsterT->m_nEquipItem1;
		m_nEquipPro2 		= 	m_pMonsterT->m_nEquipPro2;
		m_nEquipItem2 		= 	m_pMonsterT->m_nEquipItem2;
		m_nEquipPro3 		= 	m_pMonsterT->m_nEquipPro3;
		m_nEquipItem3 		= 	m_pMonsterT->m_nEquipItem3;
		m_nEquipPro4 		= 	m_pMonsterT->m_nEquipPro4;
		m_nEquipItem4 		= 	m_pMonsterT->m_nEquipItem4;
		m_nEquipPro5 		= 	m_pMonsterT->m_nEquipPro5;
		m_nEquipItem5 		= 	m_pMonsterT->m_nEquipItem5;
		m_nSpecialArtPro 	= 	m_pMonsterT->m_nSpecialArtPro;
		m_nSpecialArt 		= 	m_pMonsterT->m_nSpecialArt;
		m_nScriptID 		= 	m_pMonsterT->m_nScriptID;
	}
}

void monster::load_drop(CNetMessage &msg)
{
	txt_data_loader_ptr item_ptr(new txt_data_loader());
	int count;
	count = rand()%100000+1;
	char file_name[100];
	memset(file_name,0,sizeof(file_name));
	/*_snprintf(file_name,sizeof(file_name),"..//..//release//JLinData//Data//DropData//100001.txt",m_nEquipItem1);
	item_ptr->fetch_data(file_name,pack);	*/

	//-----------------------------------这是策划所要求的必爆一个物品
	if (count<m_nEquipPro1)
	{
		_snprintf(file_name,sizeof(file_name),"..//..//..//release//JLinData//Data//DropData//%d.txt",m_nEquipItem1);
		item_ptr->fetch_data(file_name,msg);	
	}

	if (count<(m_nEquipPro2+m_nEquipPro1)&&count>m_nEquipPro1)
	{
		_snprintf(file_name,sizeof(file_name),"..//..//..//release//JLinData//Data//DropData//%d.txt",m_nEquipItem2);
		item_ptr->fetch_data(file_name,msg);
	}
	if (count<(m_nEquipPro3+m_nEquipPro2+m_nEquipPro1)&&count>(m_nEquipPro2+m_nEquipPro1))
	{
		_snprintf(file_name,sizeof(file_name),"..//..//..//release//JLinData//Data//DropData//%d.txt",m_nEquipItem3);
		item_ptr->fetch_data(file_name,msg);

	}
	if (count<(m_nEquipPro4+m_nEquipPro3+m_nEquipPro2+m_nEquipPro1)&&count>(m_nEquipPro3+m_nEquipPro2+m_nEquipPro1))
	{
		_snprintf(file_name,sizeof(file_name),"..//..//..//release//JLinData//Data//DropData//%d.txt",m_nEquipItem4);
		item_ptr->fetch_data(file_name,msg);
	}
	if (count<(m_nEquipPro5+m_nEquipPro4+m_nEquipPro3+m_nEquipPro2+m_nEquipPro1)&&count>(m_nEquipPro4+m_nEquipPro3+m_nEquipPro2+m_nEquipPro1))
	{
		_snprintf(file_name,sizeof(file_name),"..//..//..//release//JLinData//Data//DropData//%d.txt",m_nEquipItem5);
		item_ptr->fetch_data(file_name,msg);
	}
	msg<<m_nEXP<<m_nMoney;
}


void monster::ProcessMsg(CNetMessage &msg,int sock)
{
	/*int type = msg.GetType();
	switch(type)
	{
	case:
		break;
	default:
		break;
	}*/
}
void monster::AI_OnLoop(DWORD dwTime)
{
	COnlineUserList* userlist = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if( m_pMonsterT == NULL ) return;
again:
	switch ( m_state )
	{
	case WANDER://闲置
		{
			m_pTarget = NULL;
			if ( IsActivePursuit() )//如果是主动追击型
			{
				list<int> socklist = m_pScene->GetPlayerSocks();
				for ( list<int>::iterator it = socklist.begin(); it != socklist.end(); it++ )
				{
					CPlayer* player = userlist->GetUserBySock(*it);
					if ( player )
					{
						if(  IsPlayerInSpyRange(player) && !player->IsDead() )//侦测到攻击目标(活的)
						{
							m_pTarget = player;
							ChangeFaceToPos(m_pTarget->GetPosition());
							m_state = BATTLEMOVE;
							goto again;
						}
					}
				}
			}
			else//如果是被动追击型
			{
				if ( m_bBeAttack && m_pAttacker && !m_pAttacker->IsDead() )//检查到玩家攻击
				{
					m_pTarget = m_pAttacker;
					ChangeFaceToPos(m_pTarget->GetPosition());
					m_state = BATTLE;
					goto again;
				}
			}
			AI_WalkOneStep(dwTime);
		}
		break;
	case BATTLEMOVE://战斗移动
		{
			if ( IsPlayerInSpyRange( m_pTarget ) && !m_pTarget->IsDead() )//如果目标在侦测范围内且未死
			{
				ChangeFaceToPos(m_pTarget->GetPosition());
				if( IsTargetInAttRange() )//如果目标在攻击范围内，转入攻击状态
				{
					m_state = BATTLE;
					goto again;
				}
				else //如果目标不在攻击范围内，则追击
				{
					AI_PursuitOneStep(dwTime);
				}
			}
			else//如果目标不在侦测范围内，则转入闲置状态
			{
				m_bBeAttack = false;
				m_pAttacker = NULL;
				m_state = WANDER;
				ChangeFaceToPos(originPos);
				goto again;
			}
		}
		break;
	case BATTLE://战斗
		if ( IsTargetInAttRange() )//如果目标在攻击范围内，继续攻击
		{
			if ( !m_pTarget->IsDead() )
			{
				ChangeFaceToPos(m_pTarget->GetPosition());
				if ( CheckTimeExpired(dwTime) )
				{
					SendMovement(MONSTER_ATT);
					if( CheckCrash() && CheckAccuracy() )//碰撞及命中检测
					{
						AI_Attack();
					}
				}
				m_lasttime = dwTime;
			}
			else//如果目标被打死则停止攻击，转入闲置状态
			{
				m_bBeAttack = false;
				m_pAttacker = NULL;
				m_state = WANDER;
				goto again;
			}
		}
		else//如果目标不在攻击范围内
		{
			if ( IsPlayerInSpyRange(m_pTarget) && !m_pTarget->IsDead() )//如果目标在侦测范围内，且未死，转入追击状态
			{
				m_state = BATTLEMOVE;
				goto again;
			}
			else//如果目标离开侦测范围，则转入闲置状态
			{
				m_bBeAttack = false;
				m_pAttacker = NULL;
				m_state = WANDER;
				ChangeFaceToPos(originPos);
				goto again;
			}
		}
		break;
	default:
		break;
	}
}

void monster::AI_GoOneStep(float speed,DWORD dwTime)
{
	if( m_pMonsterT == NULL ) return;
	float deltax,deltaz;
	float sx,sy,sz,cursx,cursz;
	float& x = pos.x_;
	float& y = pos.y_;
	float& z = pos.z_;
	sx=originPos.x_-x;
	sy=originPos.y_-y;
	sz=originPos.z_-z;
	cursx=cosf(face_*(PI/4));
	cursz=sinf(face_*(PI/4));
	//检查是否走到巡逻边界
	if( (sx*sx+sy*sy+sz*sz) >= (float)pow((float)m_pMonsterT->m_nMoveRange,2)*10000.0f)
	{
		if(cursx*sx+cursz*sz<0)//判断是否往回走,不是则需转身
		{
			face_ += 4;
			face_ %= 8;
			cursx = -cursx;
			cursz = -cursz;
		}
	}
	deltax=cursx*speed*(float)(dwTime-m_lasttime)MSEC/1000.0f;
	deltaz=cursz*speed*(float)(dwTime-m_lasttime)MSEC/1000.0f;
	float xsize,zsize;
	TerrainInfo terInfo;
	if( m_pScene && server_data::instance().GetTerrainInfo(m_pScene->map_name_ ,terInfo) )
	{
		xsize = terInfo.m_Width*terInfo.m_TileSize;
		zsize = terInfo.m_Height*terInfo.m_TileSize;
	}
	else
		return;
	if( x+deltax <= 0 || x+deltax >= xsize || z+deltaz <= 0 || z+deltaz >= zsize || 
		server_data::instance().IsObstruct(m_pScene->map_name_,(int)(x+deltax),(int)(z+deltaz)) )
	{
		face_++;
		face_ %= 8;
	}
	x += deltax;
	z += deltaz;
	if( m_pTarget == NULL )
		SendMovement( MONSTER_WALK );
	else
		SendMovement( MONSTER_RUN );
	m_lasttime = dwTime;
}

void monster::AI_WalkOneStep(DWORD dwTime)
{
	AI_GoOneStep(GetMoveSpeed(),dwTime);
}

void monster::AI_PursuitOneStep(DWORD dwTime)
{
	AI_GoOneStep(GetPursuitSpeed(),dwTime);
}

void monster::SendMovement( EMonsterSeq movement )
{
	float speed = 0.0;
	switch ( movement )
	{
	case MONSTER_WALK:
		speed = GetMoveSpeed();
		break;
	case MONSTER_RUN:
		speed = GetPursuitSpeed();
		break;
	case MONSTER_ATT:
		speed = GetAttSpeed();
		break;
	default:
		break;
	}
	list<int> socklist = m_pScene->GetPlayerSocks();
	for ( list<int>::iterator it = socklist.begin(); it != socklist.end(); it++ )
	{
		CNetMessage msg;
		msg.SetType(GC_MONSTER_MOVE);
		msg << insid << movement << pos.x_ << pos.y_ << pos.z_ << face_ << speed; 
		CGameSocket::instance().SendMsg(*it,msg);
	}
}

float monster::CalcDistance(vector3d src,vector3d dest)
{
	float dx = src.x_ - dest.x_;
	float dy = src.y_ - dest.y_;
	float dz = src.z_ - dest.z_;
	return sqrt( pow(dx,2)+pow(dy,2)+pow(dz,2) );
}

bool monster::IsPlayerInSpyRange(CPlayer* player)
{
	if( player )
		return (CalcDistance(player->GetPosition(),pos) < m_pMonsterT->m_nSpyRange*sqrt(200.0));
	return false;
}

bool monster::CalcAttPoint(vector3d& point)
{
	if ( m_pTarget )
	{
		float cursx = cosf((float)face_*PI/4.0f);
		float cursz = sinf((float)face_*PI/4.0f);
		vector3d target = m_pTarget->GetPosition();
		float dist = CalcDistance(pos,target) - m_pMonsterT->m_nAttackDistance;
		point = pos;
		point.x_ += dist*cursx;
		point.z_ += dist*cursz;
		return true;
	}
	return false;
}

void monster::ChangeFaceToPos(const vector3d& targetpos )
{
	float distx = targetpos.x_ - pos.x_;
	float distz = targetpos.z_ - pos.z_;
	const float EPSINON = 50.0;
	if ( abs(distz) <= EPSINON )
	{
		if ( distx > 0 )
			face_ = RIGHT;
		else
			face_ = LEFT;
		return;
	}
	if ( abs(distx) <= EPSINON )
	{
		if ( distz > 0 )
			face_ = UP;
		else
			face_ = DOWN;
		return;
	}
	if ( distx > 0 && distz > 0 )
	{
		face_ = RIGHT_UP;
	}
	if ( distx < 0 && distz > 0 )
	{
		face_ = LEFT_UP;
	}
	if ( distx < 0 && distz < 0 )
	{
		face_ = LEFT_DOWN;
	}
	if ( distx > 0 && distz < 0 )
	{
		face_ = RIGHT_DOWN;
	}
}

bool monster::IsTargetInAttRange()
{
	if ( m_pTarget )
	{
		if( CalcDistance(pos,m_pTarget->GetPosition()) <= m_pMonsterT->m_nAttackDistance*30  )
			return true;
		else
			return false;
	}
	return false;
}

bool monster::IsActivePursuit()
{
	if ( m_pMonsterT && m_pMonsterT->m_ePursuitType == Logic::MonsterAtt_PursuitRange_1 )
		return true;
	return false;
}

void monster::BeAttack(CPlayer* player)
{
	m_bBeAttack = true;
	m_pAttacker = player;
}

void monster::AI_Attack()
{
	if ( m_pTarget )
	{
		int reduceHP = 10;
		CNetMessage msg;
		m_pTarget->SetHP(m_pTarget->GetHP()-reduceHP);
		if ( m_pTarget->GetHP() <= 0 )
		{
			m_pTarget->SetHP(0);
			msg.SetType(GC_PLAYER_HP);
			msg << reduceHP << m_pTarget->GetHP();
			CGameSocket::instance().SendMsg(m_pTarget->GetSock(),msg);
			msg.ReWrite();
			msg.SetType(GC_PLAYER_DIE);
			CGameSocket::instance().SendMsg(m_pTarget->GetSock(),msg);
		}
		else
		{
			msg.SetType(GC_PLAYER_HP);
			msg << reduceHP << m_pTarget->GetHP();
			CGameSocket::instance().SendMsg(m_pTarget->GetSock(),msg);
		}
	}
}

bool monster::CheckCrash()
{
	if ( m_pTarget )
	{
		Circle cMonster,cPlayer;
		cMonster.center_ = Point(pos.x_,pos.z_);
		cMonster.radius_ = m_nModelSize;
		cPlayer.center_ = Point(m_pTarget->GetPosition().x_,m_pTarget->GetPosition().z_);
		cPlayer.radius_ = m_pTarget->GetModelSize();
		float d1 = cMonster.center_.x_-cPlayer.center_.x_;
		float d2 = cMonster.center_.y_-cPlayer.center_.y_;
		if ( sqrt(pow(d1,2)+pow(d2,2)) <= cMonster.radius_+cPlayer.radius_ )
			return true;
	}
	return false;
}

bool monster::CheckAccuracy()
{
#define MAX_RANDOM 10
	if ( m_pTarget )
	{
		if( m_pTarget->GetHitRate() >= 1.0 )
			return true;
		float r = (rand()%(int)((1.0-0.0)*MAX_RANDOM))/(float)MAX_RANDOM+0.0;
		if( r >= m_pTarget->GetHitRate() )
			return true;
	}
	return false;
}

bool monster::CheckTimeExpired(DWORD dwTime)
{
	if( (0 == m_lastHitTime) || (dwTime - m_lastHitTime >= m_dwHitIntervalTime) )
	{
		m_lastHitTime = dwTime;
		return true;
	}
	return false;
}