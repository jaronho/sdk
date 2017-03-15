/******************************************************************************
* 作者: lf
* 时间: 2010-4-20
* 描述: 逻辑相关的表格
******************************************************************************/
#ifndef LogicTable_H
#define LogicTable_H

#include "EQMainPrerequisites.h"

/**
*/
struct HelpBtnShowPicRow 
{
	HelpBtnShowPicRow()
		:id(0)
	{

	}
	int id;
	std::string btn_name;
	std::string picID;
};
typedef stdext::hash_map<uint, HelpBtnShowPicRow>		HelpBtnShowPicTable; 

/**大表情结构
*/
struct BigExpressionRow 
{
	BigExpressionRow()
		:id(0)
		,playtime(0)
	{
	}

	int id;
	std::string transferred;
	int playtime;
	std::string firstf;
	std::string secondf;
	std::string thirdf;
	std::string forthf;
	std::string fifthf;
	std::string sixthf;
	std::string seventhf;
	std::string eightf;
	std::string ninthf;
	std::string tenthf;
};
typedef stdext::hash_map<uint, BigExpressionRow>		BigExpressionTable; 

/** 家具功能
*/
struct FurniInteractRow
{
	FurniInteractRow()
		: id( 0 )
		, time( 0 )
		, voiceid( 0 )
	{
	}

	uint			id;
	std::string 	name;
	uint			type;			//1：结束交互时家具停止家具动画，2：每次交互轮流开/关家具动画
	uint			furni_ani;		//家具动画id
	uint			furni_act;		//家具行为id
	std::vector<int>			menu_info;		//家具交互菜单信息，有多个家具动画时为2个int，当前动画编号/最大动画编号
	std::string		role_ani;		//人物骨骼动画
	DWORD			time;			//家具交互持续时间
	uint			hide_slot;		//隐藏部件
	std::vector<Ogre::Vector3>	mosaic;			//马赛克位置
	std::vector<Ogre::Vector3>	offset_male;	//人物坐标偏移，相对于家具坐标的偏移(男)
	std::vector<Ogre::Vector3>	offset_female;	//人物坐标偏移，相对于家具坐标的偏移(女)
	uchar			face_male;		//人物朝向，相对于家具朝向的朝向(男)
	uchar			face_female;	//人物朝向，相对于家具朝向的朝向(女)
	uint			use_range;		//家具使用距离
	uchar			use_face;		//家具使用面朝向
	uint			max_user;		//人数限制
	std::string		sound;			//动作音效
	std::string		imageset;		//菜单上的icon
	std::string		icon;			//菜单上的icon
	std::string		tooltip;		//icon的提示
	std::string		start_hint;		//动作开始提示
	std::string		end_hint;		//动作结束提示
	uint			voiceid;
};
typedef stdext::hash_map<uint, FurniInteractRow>		FurniInteractTable; 

struct FurniAnimationRow
{
	uint				id;
	std::string 		name;
	std::vector<int>	type;				//形变、纹理、骨骼、粒子、灯光
	std::vector<int>	morph_frame;		//家具关键帧
	std::string			texture_express;	//纹理动画表达式
	std::string			skeleton_name;		//骨骼动画名
	bool				skeleton_loop;		//骨骼动画时间
	uint				particle;			//粒子
};
typedef stdext::hash_map<uint, FurniAnimationRow>		FurniAnimationTable; 

struct FurniActionRow
{
	uint		id;
	std::string	name;
	uint		type;
	std::string	param;
};
typedef stdext::hash_map<uint, FurniActionRow>		FurniActionTable; 

/** 环形菜单
*/
struct LoopMenuRow 
{
	LoopMenuRow()
		: id( 0 )
	{
	}

	uint			id;
	uint			sex;
	uint			fun1;
	uint			fun2;
	uint			fun3;
	uint			fun4;
	uint			fun5;
	uint			fun6;
	uint			fun7;
	uint		hint;		//提示
	uint			authority;	//权限
};
typedef stdext::hash_map<uint, LoopMenuRow>		LoopMenuTable; 

/** 商城、商店
*/
struct ShopCenterRow
{
	uint			good_id;
	uint			item_id;
	uint			type;
	int				sex;
	int				money_type;
	uint			price;
	std::string		intro;
	bool			recommend;//商城专用
	uint			sale_type;//商店专用
};
typedef stdext::hash_map<uint, ShopCenterRow>	ShopCenterTable;

struct ShopCenterIconRow
{
	uint			type;
	std::string		imageset;
	std::string		normal;
	std::string		push;
	std::string		hover;
	std::string		tooltip;
};
typedef stdext::hash_map<uint, ShopCenterIconRow>	ShopCenterIconTable;

struct NpcShopMgrRow
{
	NpcShopMgrRow()
		: page1(false),page2(false),page3(false),page4(false),page5(false),default_page(0)
	{
	}
	uint				id;
	std::vector<uint>	sale_type;
	bool				page1;
	bool				page2;
	bool				page3;
	bool				page4;
	bool				page5;
	int					default_page;
	std::string			shop_name;
};
typedef stdext::hash_map<uint, NpcShopMgrRow>		NpcShopMgrTable; 

struct HouseSalesRow
{
	HouseSalesRow()
		:indexId(0)
		,name("")
		,type(0)
		,recommend(0)
		,level(0)
		,payType(0)
		,price(0)
		,houseId(0)
		,descriptInfo("")
		,cost_flower(0)
		,need_decoration(0)
	{

	}
	uint		indexId;
	std::string	name;
	uint		type;
	bool        recommend;
	int			level;
	int			payType;
	int			price;
	int			houseId;
	std::string	descriptInfo;
	int			cost_flower;
	int			need_decoration;
};
typedef stdext::hash_map<uint, HouseSalesRow>	HouseSalesTable;
// 家政表
struct DoServiceRow
{
	DoServiceRow()
		:id(0),
		name(""),
		propertyId(0),
		addValue(0),
		image("")
	{

	}
	uint id;
	uint propertyId;
	uint addValue;
	std::string name;
	std::string image;
};
typedef stdext::hash_map<uint, DoServiceRow>	DomesticServiceTable;
// 打工种类表
struct WorkRow
{
	WorkRow()
		:id(0)
		,grade(0)
		,money(0)
		,clean(0)
		,health(0)
		,charm(0)
		,active(0)
		,healthState(0)
		,propertyid(0)
		,workImage("")
		,typeImageSet("")
		,typeImage("")
	{

	}
	uint id;
	uint grade;
	int  money;
	int  clean;
	int  health;
	int  charm;
	int  active;
	int  healthState;
	uint propertyid;
	std::string workImage;
	std::string typeImageSet;
	std::string typeImage;
};
typedef stdext::hash_map<uint, WorkRow>	WorksTable;

// 人物属性
struct PlayerPropertyRow
{
	PlayerPropertyRow()
		:id(0)
		,clean(0)
		,health(0)
		,charm(0)
		,active(0)
	{

	}
	uint id;
	int  clean;
	int  health;
	int  charm;
	int  active;
};
typedef stdext::hash_map<uint, PlayerPropertyRow>	PlayerPropertyTable;

// 打工特殊事件
struct WorkEventRow
{
	WorkEventRow()
		:id(0)
		,image("")
	{

	}
	uint id;
	std::string image;
};
typedef stdext::hash_map<uint, WorkEventRow> WorkEventTable;
//传送点
struct TeleportRow 
{
	uint id;
	uint src_scene_id;
	float src_x,		
		 src_y,		
		 src_z,		
		 src_radius;
};
typedef stdext::hash_map<int, TeleportRow> TeleportTable; 

// 场景信息表
struct SceneRow
{
	uint				id;
	std::string			name;
	uint				type;
	std::string			file;
	float				far_clip_distance;

	int 				pitchMin, 
						pitchMax, 
						yawMin, 	 
						yawMax,	
						zoomMin,	
						zoomMax;
	Ogre::Vector3		lookatMin,
						lookatMax;
	int					voiceid;

};
typedef stdext::hash_map<uint, SceneRow> SceneTable; 

// 出生点表
struct BornRow
{
	uint				id;
	Ogre::Vector3		pos;
	int					dir;
	Ogre::Vector3		lookat;
	int					pitch;
	int					yaw;
	int					zoom;
};
typedef stdext::hash_map<uint, BornRow> BornTable;

// 家政价格表
struct ServicePriceRow 
{
	ServicePriceRow()
		:price(0)
		,level(0)
	{

	}
	int level;
	uint price;
};
typedef stdext::hash_map<uint, ServicePriceRow> ServicePriceTable; 

// loading表
struct LoadingRow
{
	std::string set;
	std::string image;
};
typedef std::vector<LoadingRow> LoadingTable;
typedef std::vector<std::string> LoadingTipsTable;

// 打开界面时打开或关闭响应界面表
struct OpenUIRow
{
	OpenUIRow()
		:name("")
		,relationUISets("")
		,mutexUISets("")
	{

	}

	std::string name;
	std::string relationUISets;
	std::string mutexUISets;
};
typedef stdext::hash_map<std::string , OpenUIRow> OpenUITable;
// 礼物盒样式
struct GiftBoxRow
{
	GiftBoxRow()
		:id(0)
		,type(0)
		,gameGold(0)
		,eqGold(0)
		,modelId(0)
		,name("")
		,imageset("")
		,image("")
		,desc("")
	{

	}

	uint id;
	uint type;	
	uint gameGold;
	uint eqGold;
	uint modelId;
	std::string name;
	std::string imageset;
	std::string image;
	std::string desc;
};
typedef stdext::hash_map<uint , GiftBoxRow> GiftBoxTable;
// 礼物卡样式
struct GiftCardRow
{
	GiftCardRow()
		:id(0)
		,type(0)
		,gameGold(0)
		,eqGold(0)
		,name("")
		,imageset("")
		,image("")
		,desc("")
		,urlAddress("")
	{

	}

	uint id;
	uint type;	
	uint gameGold;
	uint eqGold;
	std::string name;
	std::string imageset;
	std::string image;
	std::string desc;
	std::string urlAddress;
};
typedef stdext::hash_map<uint , GiftCardRow> GiftCardTable;

// 路标
struct GotoRow
{
	GotoRow()
		:id(0)
		,page(0)
		,pos(0)
		,image("")
		,name("")
		,target(0)
	{

	}

	uint id;
	uint page;
	uint pos;
	std::string image;
	std::string name;
	uint target;
};
typedef stdext::hash_map<uint, GotoRow> GotoTable;

// 物品说明表(add by hezhr)
struct GoodsDescribeRow
{
	GoodsDescribeRow():id(0),value("") {}
	uint id;
	std::string value;
};
typedef stdext::hash_map<uint, GoodsDescribeRow> GoodsDescribeTable;

// 玩家向导(add by hezhr)
struct PlayerGuideRow
{
	PlayerGuideRow():id(0),row(0),col(0) {}
	uint id;
	uint row;
	uint col;
	std::vector<std::string> imageset;
	std::vector<std::string> image;
};
typedef stdext::hash_map<uint, PlayerGuideRow> PlayerGuideTable;

// 新手教学步骤表(add by hezhr)
struct NoviceTeachStepRow
{
	NoviceTeachStepRow():step(0),content(""),nextBtnstr(""),preBtnstr("") {}
	uint step;
	std::string content;
	std::string nextBtnstr;
	std::string preBtnstr;
};
typedef stdext::hash_map<uint, NoviceTeachStepRow> NoviceTeachStepTable;

// 互斥父窗口索引表
struct MutexParentIdxRow
{
	MutexParentIdxRow()
		:id(0)
		,parent(-1)
		,name("")
	{

	}

	uint id;
	int parent;
	std::string name;
};
typedef stdext::hash_map<uint, MutexParentIdxRow> MutexParentIdxTable;

// 互斥界面表
struct MutexUIRow
{
	MutexUIRow()
		:name("")
		,relation("")
		,mutex("")
	{

	}

	std::string name;
	std::string relation;
	std::string mutex;
};
typedef stdext::hash_map<std::string, MutexUIRow> MutexUITable;

// 宝箱
struct MagicBoxRow
{
	MagicBoxRow()
		:id(0)
		,modelId(0)
	{

	}

	uint id;
	uint modelId;
};
typedef stdext::hash_map<uint, MagicBoxRow> MagicBoxTable;

// 垃圾
struct GarbageRow
{
	GarbageRow()
		:id(0)
		,modelId(0)
	{

	}

	uint id;
	uint modelId;
};
typedef stdext::hash_map<uint, GarbageRow> GarbageTable;

// 声音
struct VoiceRow
{
	VoiceRow()
		:id(0)
		,cls(0)
		,ltype(0)
		,file("")
	{

	}

	uint id;
	int  cls;
	int  ltype;
	std::string file;
};
typedef stdext::hash_map<uint, VoiceRow> VoiceTable;

// 窗体声音
struct UIVoiceRow
{
	UIVoiceRow()
		:id(0)
		,hovervid(0)
		,clickvid(0)
		,name("")
	{

	}

	uint id;
	uint hovervid;
	uint clickvid;
	std::string name;
};
typedef stdext::hash_map<std::string, UIVoiceRow> UIVoiceTable;
typedef UIVoiceTable::const_iterator UIVoiceCIter;

// 特定动作声音
struct SpecialVoiceRow
{
	SpecialVoiceRow()
		:id(0)
		,voiceid(0)
		,name("")
	{

	}

	uint id;
	uint voiceid;
	std::string name;
};
typedef stdext::hash_map<uint, SpecialVoiceRow> SpecialVoiceTable;

// 疾病表
struct DiseaseRow
{
	DiseaseRow()
		:health(0)
		,particle(0)
	{
	}

	uint health;
	uint particle;
};
typedef stdext::hash_map<uint, DiseaseRow> DiseaseTable;

// 疾病特殊事件
struct DiseaseSpecialEventRow
{
	DiseaseSpecialEventRow()
		:id(0)
		,pid(0)
		,images("")
	{

	}

	uint id;
	uint pid;
	std::string images;
};
typedef stdext::hash_map<uint, DiseaseSpecialEventRow> DiseaseSpecialEventTable;

// 更换墙壁地板材质表(add by hezhr)
struct ChangeWallFloorMaterialRow
{
	ChangeWallFloorMaterialRow()
		:id(0)
		,type(0)
		,name("")
		,material("")
	{

	}

	uint id;
	uint type;
	std::string name;
	std::string material;
};
typedef stdext::hash_map<uint, ChangeWallFloorMaterialRow> ChangeWallFloorMaterialTable;

// 迷你地图表(2011/7/28 add by hezhr)
struct MiniMapRow
{
	MiniMapRow()
		:scene_id(0)
		,row(0)
		,col(0)
	{

	}

	uint scene_id;
	uint row;
	uint col;
	std::vector<std::string> imageset;
	std::vector<std::string> image;
};
typedef stdext::hash_map<uint, MiniMapRow> MiniMapTable;


class LogicTableManager : public Ogre::Singleton<LogicTableManager>
{
public:
	void load();

	const FurniInteractRow*		getFurniInteractRow( uint id ) const; 
	const FurniInteractTable&	getFurniInteractTable() const { return mFurniInteractTable; }

	const FurniAnimationRow*	getFurniAnimationRow( uint id ) const; 
	const FurniAnimationTable&	getFurniAnimationTable() const { return mFurniAnimationTable; }

	const FurniActionRow*		getFurniActionRow( uint id ) const; 
	const FurniActionTable&		getFurniActionTable() const { return mFurniActionTable; }

	const LoopMenuRow*			getLoopMenuRow( uint id ) const; 
	const LoopMenuTable&		getLoopMenuTable() const { return mLoopMenuTable; }

	const ShopCenterRow*		getShopCenterRow( uint id ) const; 
	const ShopCenterRow*		getShopCenterRowByItemID( uint itemid ) const; //很搓，没办法了 
	const ShopCenterTable&		getShopCenterTable() const { return mShopCenterTable; }

	const ShopCenterIconRow*	getShopCenterIconRow( uint id ) const; 
	const ShopCenterIconRow*	getFitModeShopCenterIconRow( uint id ) const; 

	const ShopCenterRow*		getNpcShopRow( uint id ) const; 
	const ShopCenterRow*		getNpcShopRowByItemID( uint itemid ) const; //很搓，没办法了
	const ShopCenterTable&		getNpcShopTable() const { return mNpcShopTable; }

	const HouseSalesTable&		getHouseSalesTable() const{return mHouseSalesTable;}
	const HouseSalesRow*		getHouseSalesRow(uint id) const;

	const DomesticServiceTable& getDoServiceTable() const{return mDoserviceTable;}
	const DoServiceRow*			getDoserviceRow(uint id) const;

	const WorksTable&			getWorksTable() const {return mWorkTable;}
	const WorkRow*				getWorkRow(uint id) const;
	
	const PlayerPropertyTable&	getPlayerPropertyTable() const {return mPlayerPropertyTable;}
	const PlayerPropertyRow*	getPlayerPropertyRow(uint id) const;

	const WorkEventTable&		getWorkEventTable(){return mWorkEventTable;}
	const WorkEventRow*			getWorkEventRow(uint id) const;

	const SceneRow*				getSceneRow(uint id) const;

	BornRow*					getBornRow(uint id) const;

	const TeleportRow*			getTeleportRow(uint id) const;
	const TeleportTable&		getTeleportTable() const{return mTeleportTable;}

	const ServicePriceRow*      getServicePriceRow(uint level) const;

	const LoadingRow*			getRandomLoadingRow() const;
	std::string					getRandomLoadingTips() const;

	const OpenUIRow*			getOpenUIRow(const std::string winName) const;

	const GiftBoxTable&			getGiftBoxTable() const{ return mGiftBoxTable;}
	const GiftBoxRow*			getGiftBoxRow(uint id) const;

	const GiftCardTable&		getGiftCardTable() const {return mGiftCardTable;}
	const GiftCardRow*			getGiftCardRow(uint cardId) const;

	const GotoTable&			getGotoTable() const {return mGotoTable;}
	const uint					getGotoRows() const {return mGotoTable.size();}
	const GotoRow*				getGotoRow(uint id) const;
	const GotoRow*				getGotoRowByName(std::string name) const;
	std::string					getGotoUserData(std::string acc, uint type);
	void						setGotoUserData(std::string acc, uint type, std::string data);

	const GoodsDescribeRow*		getGoodsDescribeRow(uint id) const;
	const GoodsDescribeTable&	getGoodsDescribeTable() const {return mGoodsDescribeTable;}

	const PlayerGuideRow*		getPlayerGuideRow(uint id) const;
	const PlayerGuideTable&		getPlayerGuideTable() const {return mPlayerGuideTable;}
	std::string					getPlayerGuideImageset(uint id, uint i);
	std::string					getPlayerGuideImage(uint id, uint i);

	const NoviceTeachStepRow*	getNoviceTeachStepRow(uint step) const;
	const NoviceTeachStepTable&	getNoviceTeachStepTable() const {return mNoviceTeachStepTable;}

	const MutexParentIdxRow*    getMutexParentIdxRow(uint id) const;
	const MutexParentIdxRow*	getMutexParentIdxRowN(const std::string name) const;
	const uint				    getMutexParentIdxRows() const {return mMutexParentIdxTable.size();}
	const MutexParentIdxTable&  getMutexParentIdxTable() const {return mMutexParentIdxTable;}

	const MutexUIRow*           getMutexUIRow(const std::string name) const;
	const uint				    getMutexUIRows() const {return mMutexUITable.size();}
	const MutexUITable&         getMutexUITable() const {return mMutexUITable;}

	const NpcShopMgrRow*		getNpcShopMgrRow(uint id) const;

	const MagicBoxRow*			getMagicBoxRow(uint id) const;
	const uint					getMagicBoxRows() const {return mMagicBoxTable.size();}
	const MagicBoxTable&		getMagicBoxTable() const {return mMagicBoxTable;}

	const GarbageRow*			getGarbageRow(uint id) const;
	const uint					getGarbageRows() const {return mGarbageTable.size();}
	const GarbageTable&			getGarbageTable() const {return mGarbageTable;}

	const VoiceTable&			getVoiceTable() const {return mVoiceTable;}
	const uint					getVoiceRows() const {return mVoiceTable.size();}
	const VoiceRow*				getVoiceRow(uint id) const;

	const UIVoiceTable&			getUIVoiceTable() const {return mUIVoiceTable;}
	const uint					getUIVoiceRows() const {return mUIVoiceTable.size();}
	const UIVoiceRow*			getUIVoiceRow(const std::string name) const;
	const UIVoiceRow*			getUIVoiceRowByIdx(uint idx) const;
	void						getUIVoiceRowByIdxStart();
	const UIVoiceRow*			getUIVoiceRowByIdxNext();

	const SpecialVoiceTable&	getSpecialVoiceTable() const {return mSpecialVoiceTable;}
	const uint					getSpecialVoiceRows() const {return mSpecialVoiceTable.size();}
	const SpecialVoiceRow*		getSpecialVoiceRow(uint id) const;

	const DiseaseTable &getDiseaseTable() const {return mDiseaseTable;}
	const DiseaseRow* getDiseaseRow(uint id) const;

	const DiseaseSpecialEventTable &getDiseaseSpecialEventTable() const {return mDiseaseSpecialEventTable;}
	const uint getDiseaseSpecialEventRows() const {return mDiseaseSpecialEventTable.size();}
	const DiseaseSpecialEventRow* getDiseaseSpecialEventRow(uint id) const;

	const ChangeWallFloorMaterialTable& getChangeWallFLoorMaterailTable() const {return mChangeWallFloorMaterialTable;}
	const ChangeWallFloorMaterialRow* getChangeWallFloorMaterialRow(uint id) const;
	const BigExpressionRow* getBigExpressionRow(int id) const;
	int getBigExpressionTableCount(){return mBigExpressionTable.size();}
	const HelpBtnShowPicRow* getHelpBtnShowPicRow(int id) const;
	int getHelpBtnShowPicTableCount(){return mHelpBtnShowPicTable.size();}

	const MiniMapRow*			getMiniMapRow(uint scene_id) const;
	const MiniMapTable&			getMiniMapTable() const {return mMiniMapTable;}
	std::string					getMiniMapImageset(uint scene_id, uint i);
	std::string					getMiniMapImage(uint scene_id, uint i);

private:	
	FurniInteractTable	mFurniInteractTable; 
	FurniAnimationTable	mFurniAnimationTable; 
	FurniActionTable	mFurniActionTable; 
	LoopMenuTable		mLoopMenuTable;
	ShopCenterTable		mShopCenterTable;
	ShopCenterIconTable	mShopCenterIconTable;
	ShopCenterIconTable	mFitModeShopCenterIconTable;
	ShopCenterTable		mNpcShopTable;
	HouseSalesTable		mHouseSalesTable;
	DomesticServiceTable mDoserviceTable;
	WorksTable			mWorkTable;
	PlayerPropertyTable mPlayerPropertyTable;
	WorkEventTable		mWorkEventTable;
	SceneTable			mSceneTable;
	BornTable			mBornTable;
	TeleportTable		mTeleportTable;
	ServicePriceTable   mServicePriceTable;
	LoadingTable		mLoadingTable;
	LoadingTipsTable	mLoadingTipsTable;
	OpenUITable			mOpenUITable;
	GiftBoxTable		mGiftBoxTable;
	GiftCardTable       mGiftCardTable;
	GotoTable			mGotoTable;
	GoodsDescribeTable	mGoodsDescribeTable;
	PlayerGuideTable	mPlayerGuideTable;
	NoviceTeachStepTable mNoviceTeachStepTable;
	MutexParentIdxTable mMutexParentIdxTable;
	MutexUITable        mMutexUITable;
	NpcShopMgrTable		mNpcShopMgrTable;
	MagicBoxTable       mMagicBoxTable;
	GarbageTable        mGarbageTable;
	VoiceTable			mVoiceTable;
	UIVoiceTable		mUIVoiceTable;
	UIVoiceCIter		mUIVoiceItr;
	SpecialVoiceTable	mSpecialVoiceTable;
	DiseaseTable		mDiseaseTable;
	DiseaseSpecialEventTable mDiseaseSpecialEventTable;
	ChangeWallFloorMaterialTable mChangeWallFloorMaterialTable;
	MiniMapTable		mMiniMapTable;
	BigExpressionTable	mBigExpressionTable;
	HelpBtnShowPicTable mHelpBtnShowPicTable;

private:
	void loadFurniInteractTable();
	void loadFurniAnimationTable();
	void loadFurniActionTable();
	void loadLoopMenuTable();
	void loadShopCenterTable();
	void loadShopCenterIconTable();
	void loadNpcShopTable();
	void loadNpcShopMgrTable();
	void loadHouseSalesTable();
	void loadDoserviceTable();
	void loadWorksTable();
	void loadWorkEventTable();
	void loadPlayerPropertyTable();
	void loadSceneTable();
	void loadBornTable();
	void loadTeleportTable();
	void loadServicePriceTable();
	void loadLoadingTable();
	void loadOpenUITable();
	void loadGiftBoxTable();
	void loadGiftCardTable();
	void loadGotoTable();
	void loadGoodsDescribeTable();
	void loadPlayerGuideTable();
	void loadNoviceTeachStepTable();
	void loadMutexParentIdxTable();
	void loadMutexUITable();
	void loadMagicBoxTable();
	void loadGarbageTable();
	void loadVoiceTable();
	void loadUIVoiceTable();
	void loadSpecialVoiceTable();
	void loadDiseaseTable();
	void loadDiseaseSpecialEventTable();
	void loadChangeWallFloorMaterialTable();
	void loadMiniMapTalbe();
	void loadBigExpressionTable();
	void loadHelpBtnShowPicTable();
};
#endif
