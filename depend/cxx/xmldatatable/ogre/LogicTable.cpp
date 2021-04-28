#include "EQMainPCH.h"

#include "LogicTable.h"
#include "CommFunc.h"
#include "CommFuncGcs.h"
#include "XmlDataCenter.h"
#include <string>


//-----------------------------------------------------------------------
template<> LogicTableManager* Ogre::Singleton<LogicTableManager>::ms_Singleton = 0;

//-----------------------------------------------------------------------
void LogicTableManager::load()
{
	loadFurniActionTable();
	loadFurniAnimationTable();
	loadFurniInteractTable();
	loadLoopMenuTable();
	loadShopCenterTable();
	loadShopCenterIconTable();
	loadNpcShopTable();
	loadNpcShopMgrTable();
	loadHouseSalesTable();
	loadDoserviceTable();
	loadWorksTable();
	loadWorkEventTable();
	loadPlayerPropertyTable();
	loadSceneTable();
	loadBornTable();
	loadServicePriceTable();
	loadLoadingTable();
	loadTeleportTable();
	//loadOpenUITable();
	loadMutexParentIdxTable();
	loadMutexUITable();
	loadGiftBoxTable();
	loadGiftCardTable();
	loadGotoTable();
	loadGoodsDescribeTable();
	loadPlayerGuideTable();
	loadNoviceTeachStepTable();
	loadMagicBoxTable();
	loadGarbageTable();
	loadVoiceTable();
	loadUIVoiceTable();
	loadSpecialVoiceTable();
	loadDiseaseTable();
	loadDiseaseSpecialEventTable();
	loadChangeWallFloorMaterialTable();
	loadMiniMapTalbe();
	loadBigExpressionTable();
	loadHelpBtnShowPicTable();
}
//-----------------------------------------------
void LogicTableManager::loadHelpBtnShowPicTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "ShowHelp.xml" ) );

	const size_t colID			= x.getColumnIndex( "id" );
	const size_t colBtnName	= x.getColumnIndex( "helpBtnName" );
	const size_t colPicID		= x.getColumnIndex( "showHelpPicID" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		HelpBtnShowPicRow r;
		r.id		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.btn_name		= x.cell( iRow, colBtnName );
		r.picID		= x.cell( iRow, colPicID);


		if ( mHelpBtnShowPicTable.find( r.id ) != mHelpBtnShowPicTable.end() )
			EQ_EXCEPT( "ShowHelp.xml ID Repeat", "loadHelpBtnShowPicTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "ShowHelp.xml ID null", "loadHelpBtnShowPicTable" );					

		mHelpBtnShowPicTable.insert( std::make_pair( r.id, r ) );
	}	}
//-----------------------------------------------
void LogicTableManager::loadBigExpressionTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "Big_Expression.xml" ) );

	const size_t colID			= x.getColumnIndex( "id" );
	const size_t colTransferred	= x.getColumnIndex( "transferred" );
	const size_t colPlaytime		= x.getColumnIndex( "playtime" );
	const size_t colFirstf		= x.getColumnIndex( "firstf" );
	const size_t colSecondf		= x.getColumnIndex( "secondf" );
	const size_t colThirdf		= x.getColumnIndex( "thirdf" );
	const size_t colForthf		= x.getColumnIndex( "forthf" );
	const size_t colFifthf		= x.getColumnIndex( "fifthf" );
	const size_t colSixthf		= x.getColumnIndex( "sixthf" );
	const size_t colSeventhf		= x.getColumnIndex( "seventhf" );
	const size_t colEightf	= x.getColumnIndex( "eighth" );
	const size_t colNinthf	= x.getColumnIndex( "ninthf" );
	const size_t colTenthf	= x.getColumnIndex( "tenthf" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		BigExpressionRow r;
		r.id		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.transferred		= x.cell( iRow, colTransferred );
		r.playtime		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colPlaytime, true ) );
		r.firstf		= x.cell( iRow, colFirstf, true );
		r.secondf		= x.cell( iRow, colSecondf, true );
		r.thirdf		= x.cell( iRow, colThirdf, true );
		r.forthf		= x.cell( iRow, colForthf, true );
		r.fifthf		= x.cell( iRow, colFifthf, true );
		r.sixthf		= x.cell( iRow, colSixthf, true );
		r.seventhf		= x.cell( iRow, colSeventhf, true );
		r.eightf	= x.cell( iRow, colEightf, true );
		r.ninthf	= x.cell( iRow, colNinthf, true );
		r.tenthf	= x.cell( iRow, colTenthf, true );


		if ( mBigExpressionTable.find( r.id ) != mBigExpressionTable.end() )
			EQ_EXCEPT( "Big_Expression.xml ID Repeat", "loadBigExpressionTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "Big_Expression.xml ID null", "loadBigExpressionTable" );					

		mBigExpressionTable.insert( std::make_pair( r.id, r ) );
	}	
}
//-----------------------------------------------------------------------
const HelpBtnShowPicRow* LogicTableManager::getHelpBtnShowPicRow(int id) const
{
	HelpBtnShowPicTable::const_iterator it = mHelpBtnShowPicTable.find( id );
	if ( it == mHelpBtnShowPicTable.end() )
	{
		return NULL;
	}	
	return &it->second;
}
//-----------------------------------------------------------------------
const BigExpressionRow* LogicTableManager::getBigExpressionRow(int id) const
{
	BigExpressionTable::const_iterator it = mBigExpressionTable.find( id );
	if ( it == mBigExpressionTable.end() )
	{
		return NULL;
	}	
	return &it->second;
}
//-----------------------------------------------------------------------
const FurniInteractRow* LogicTableManager::getFurniInteractRow( uint id ) const
{
	FurniInteractTable::const_iterator it = mFurniInteractTable.find( id );
	if ( it == mFurniInteractTable.end() )

	{
// 		std::string err = "id '' in furni_interact.xml is not exist!";
// 		char strID[10];
// 		sprintf(strID, "%d",id);
// 		err.insert(4,strID);
// 		LOGERR(err);
		return NULL;
	}	
	return &it->second;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadFurniInteractTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "furni_interact.xml" ) );

	const size_t colID				= x.getColumnIndex( "id" );
	const size_t colName			= x.getColumnIndex( "name" );
	const size_t colFurniAni		= x.getColumnIndex( "furni_ani" );
	const size_t colFurniAct		= x.getColumnIndex( "furni_act" );
	const size_t colMenuInfo		= x.getColumnIndex( "menu_info" );
	const size_t colRoleAni			= x.getColumnIndex( "role_ani" );
	const size_t colTime			= x.getColumnIndex( "time" );
	const size_t colHideSlot		= x.getColumnIndex( "hide_slot" );
	const size_t colMosaic			= x.getColumnIndex( "mosaic" );
	const size_t colOffsetMale		= x.getColumnIndex( "offset_male" );
	const size_t colOffsetFemale	= x.getColumnIndex( "offset_female" );
	const size_t colFaceMale		= x.getColumnIndex( "face_male" );
	const size_t colFaceFemale		= x.getColumnIndex( "face_female" );
	const size_t colUseRange		= x.getColumnIndex( "use_range" );
	const size_t colUseFace			= x.getColumnIndex( "use_face" );
	const size_t colMaxUser			= x.getColumnIndex( "max_user" );
	const size_t colSound			= x.getColumnIndex( "sound" );
	const size_t colImageset		= x.getColumnIndex( "imageset" );
	const size_t colIcon			= x.getColumnIndex( "icon" );
	const size_t colTooltip			= x.getColumnIndex( "tooltip" );
	const size_t colStartHint		= x.getColumnIndex( "start_hint" );
	const size_t colEndHint			= x.getColumnIndex( "end_hint" );
	const size_t voiceid			= x.getColumnIndex( "voiceid" );

	std::string str;
	std::vector<std::string> posArr; 
	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		FurniInteractRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.name			= x.cell( iRow, colName, true );
		r.furni_ani		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFurniAni ) );
		r.furni_act		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFurniAct ) );
		r.role_ani		= x.cell( iRow, colRoleAni, true );
		r.time			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colTime, true ) );
		r.hide_slot		= StringBitsetToUInt( x.cell( iRow, colHideSlot ) );
		r.face_male		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFaceMale, true ) );
		r.face_female	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFaceFemale, true ) );
		r.use_range		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colUseRange, true ) );
		r.use_face		= strtoul( x.cell( iRow, colUseFace, true ).c_str(), NULL, 2 );
		r.max_user		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colMaxUser, true ) );
		r.sound			= x.cell( iRow, colSound, true );
		r.imageset		= x.cell( iRow, colImageset, true );
		r.icon			= x.cell( iRow, colIcon, true );
		r.tooltip		= GB2312ToUTF8(x.cell( iRow, colTooltip, true ).c_str());
		r.start_hint	= GB2312ToUTF8(x.cell( iRow, colStartHint, true ).c_str());
		r.end_hint		= GB2312ToUTF8(x.cell( iRow, colEndHint, true ).c_str());
		r.voiceid		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, voiceid ) );

		str = x.cell( iRow, colMenuInfo, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()," );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				r.menu_info.push_back( Ogre::StringConverter::parseUnsignedInt(*it) );
			}
		}

 		if( r.menu_info.size() == 2 )
 		{
			const FurniAnimationRow* a = getFurniAnimationRow(r.furni_ani);
			if(!a)
				EQ_EXCEPT( "FurniInteract.xml getFurniAnimationRow() return NULL", "loadFurniInteractTable" );

 			if( Ogre::Math::Pow(r.menu_info[1]+1, r.max_user) != a->morph_frame.size() &&
 				a->morph_frame.size() != 1 )
 				EQ_EXCEPT( "FurniInteract.xml key_frame size invalid", "loadFurniInteractTable" );
 		}

		str = x.cell( iRow, colMosaic, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()" );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				r.mosaic.push_back(StringToVector3(*it));
			}
		}
		if(r.mosaic.size()!=0 && r.mosaic.size()!=2)
			EQ_EXCEPT( "FurniInteract.xml mosaic invalid", "loadFurniInteractTable" );

		str = x.cell( iRow, colOffsetMale, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()" );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				r.offset_male.push_back(StringToVector3(*it));
			}
		}
		if(r.offset_male.size() != r.max_user)
			EQ_EXCEPT( "FurniInteract.xml max_user unequal offset_male size", "loadFurniInteractTable" );

		str = x.cell( iRow, colOffsetFemale, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()" );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				r.offset_female.push_back(StringToVector3(*it));
			}
		}
		if(r.offset_female.size() != r.max_user)
			EQ_EXCEPT( "FurniInteract.xml max_user unequal offset_female size", "loadFurniInteractTable" );

		if ( mFurniInteractTable.find( r.id ) != mFurniInteractTable.end() )
			EQ_EXCEPT( "FurniInteract.xml ID Repeat", "loadFurniInteractTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "FurniInteract.xml ID null", "loadFurniInteractTable" );		

		if( r.use_face > 0xF )
			EQ_EXCEPT( "FurniInteract.xml face invalid", "loadFurniInteractTable" );
			
		mFurniInteractTable.insert( std::make_pair( r.id, r ) );
	}	
}
//-----------------------------------------------------------------------
const FurniAnimationRow* LogicTableManager::getFurniAnimationRow( uint id ) const
{
	FurniAnimationTable::const_iterator it = mFurniAnimationTable.find( id );
	if ( it == mFurniAnimationTable.end() )
		return NULL;

	return &it->second;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadFurniAnimationTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "furni_animation.xml" ) );

	const size_t colID				= x.getColumnIndex( "id" );
	const size_t colName			= x.getColumnIndex( "name" );
 	const size_t colType			= x.getColumnIndex( "type" );
	const size_t colMorthFrame		= x.getColumnIndex( "morph_frame" );
	const size_t colTextureExpress	= x.getColumnIndex( "texture_express" );
	const size_t colSkeletonName	= x.getColumnIndex( "skeleton_name" );
	const size_t colSkeletonLoop	= x.getColumnIndex( "skeleton_loop" );
	const size_t colParticle		= x.getColumnIndex( "particle" );

	std::string str;
	std::vector<std::string> posArr; 
	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		FurniAnimationRow r;
		r.id				= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.name				= x.cell( iRow, colName, true );
		r.texture_express	= x.cell( iRow, colTextureExpress, true );
		r.skeleton_name		= x.cell( iRow, colSkeletonName, true );
		r.skeleton_loop		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colSkeletonLoop ) ) != 0;
		r.particle			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colParticle ) );

		str = x.cell( iRow, colType, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()," );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				r.type.push_back( Ogre::StringConverter::parseUnsignedInt(*it) );
			}
		}

 		str = x.cell( iRow, colMorthFrame, true );
 		Ogre::StringUtil::trim( str );
 		if ( !str.empty() )
 		{
 			posArr = Ogre::StringUtil::split( str, " \r\n\t()," );
 			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
 			{
 				r.morph_frame.push_back( Ogre::StringConverter::parseUnsignedInt(*it) );
 			}
 		}

		if ( mFurniAnimationTable.find( r.id ) != mFurniAnimationTable.end() )
			EQ_EXCEPT( "furni_animation.xml ID Repeat", "loadFurniAnimationTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "furni_animation.xml ID null", "loadFurniAnimationTable" );		

		mFurniAnimationTable.insert( std::make_pair( r.id, r ) );
	}	
}
//-----------------------------------------------------------------------
const FurniActionRow* LogicTableManager::getFurniActionRow( uint id ) const
{
	FurniActionTable::const_iterator it = mFurniActionTable.find( id );
	if ( it == mFurniActionTable.end() )
		return NULL;

	return &it->second;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadFurniActionTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "furni_action.xml" ) );

	const size_t colID		= x.getColumnIndex( "id" );
	const size_t colName	= x.getColumnIndex( "name" );
	const size_t colType	= x.getColumnIndex( "type" );
	const size_t colParam	= x.getColumnIndex( "param" );

	std::string str;
	std::vector<std::string> posArr; 
	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		FurniActionRow r;
		r.id				= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.name				= x.cell( iRow, colName, true );
		r.type				= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType ) );
		r.param				= x.cell( iRow, colParam, true );

		if ( mFurniActionTable.find( r.id ) != mFurniActionTable.end() )
			EQ_EXCEPT( "furni_action.xml ID Repeat", "loadFurniActionTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "furni_action.xml ID null", "loadFurniActionTable" );		

		mFurniActionTable.insert( std::make_pair( r.id, r ) );
	}	
}
//-----------------------------------------------------------------------
const LoopMenuRow* LogicTableManager::getLoopMenuRow( uint id ) const
{
	LoopMenuTable::const_iterator it = mLoopMenuTable.find( id );
	if ( it == mLoopMenuTable.end() )
	{
// 		std::string err = "id '' in loop_menu.xml is not exist!";
// 		char strID[10];
// 		sprintf(strID, "%d",id);
// 		err.insert(4,strID);
// 		LOGERR(err);
		return NULL;
	}
	return &it->second;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadLoopMenuTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "loop_menu.xml" ) );

	const size_t colID			= x.getColumnIndex( "id" );
	const size_t colSex			= x.getColumnIndex( "sex" );
	const size_t colFun1		= x.getColumnIndex( "fun1" );
	const size_t colFun2		= x.getColumnIndex( "fun2" );
	const size_t colFun3		= x.getColumnIndex( "fun3" );
	const size_t colFun4		= x.getColumnIndex( "fun4" );
	const size_t colFun5		= x.getColumnIndex( "fun5" );
	const size_t colFun6		= x.getColumnIndex( "fun6" );
	const size_t colFun7		= x.getColumnIndex( "fun7" );
	const size_t colHint		= x.getColumnIndex( "hint" );
	const size_t colAuthority	= x.getColumnIndex( "authority" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		LoopMenuRow r;
		r.id		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.sex		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colSex ) );
		r.fun1		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun1, true ) );
		r.fun2		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun2, true ) );
		r.fun3		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun3, true ) );
		r.fun4		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun4, true ) );
		r.fun5		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun5, true ) );
		r.fun6		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun6, true ) );
		r.fun7		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colFun7, true ) );
		r.hint		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colHint, true ) );
		r.authority	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colAuthority, true ) );

		if ( mLoopMenuTable.find( r.id ) != mLoopMenuTable.end() )
			EQ_EXCEPT( "LoopMenu.xml ID Repeat", "loadLoopMenuTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "LoopMenu.xml ID null", "loadLoopMenuTable" );					

		mLoopMenuTable.insert( std::make_pair( r.id, r ) );
	}	
}
//-----------------------------------------------------------------------
const ShopCenterRow* LogicTableManager::getShopCenterRow( uint id ) const
{
	ShopCenterTable::const_iterator it = mShopCenterTable.find( id );
	if ( it == mShopCenterTable.end() )
		return NULL;
	
	return &it->second;
}
//-----------------------------------------------------------------------
const ShopCenterRow* LogicTableManager::getShopCenterRowByItemID( uint itemid ) const
{
	ShopCenterTable::const_iterator it;
	for(it=mShopCenterTable.begin(); it!=mShopCenterTable.end(); it++)
	{
		if(it->second.item_id == itemid)
			return &it->second;
	}

	return NULL;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadShopCenterTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "sys_shop_tplt.xml" ) );

	const size_t colGoodID		= x.getColumnIndex( "goods_id" );
	const size_t colItem		= x.getColumnIndex( "item_id" );
	const size_t colType		= x.getColumnIndex( "type" );
	const size_t colSex			= x.getColumnIndex( "sex" );
	const size_t colMoneyType	= x.getColumnIndex( "money_type" );
	const size_t colPrice		= x.getColumnIndex( "price" );
	const size_t colIntro		= x.getColumnIndex( "intro" );
	const size_t colRecommend	= x.getColumnIndex( "commend" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		ShopCenterRow r;
		r.good_id	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colGoodID ) );
		r.item_id	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colItem ) );
		r.type		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType, true ) );
		r.sex		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colSex, true ) );
		r.money_type= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colMoneyType, true ) );
		r.price		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colPrice, true ) );
		r.intro		= GB2312ToUTF8(x.cell( iRow, colIntro, true ).c_str());
		r.recommend	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colRecommend, true ) ) != 0;

		if ( mShopCenterTable.find( r.good_id ) != mShopCenterTable.end() )
			EQ_EXCEPT( "sys_shop_tplt.xml ID Repeat", "loadShopCenterTable" );

		if ( 0 == r.good_id )
			EQ_EXCEPT( "sys_shop_tplt.xml ID null", "loadShopCenterTable" );					

		mShopCenterTable.insert( std::make_pair( r.good_id, r ) );
	}	
}
//-----------------------------------------------------------------------
const ShopCenterIconRow* LogicTableManager::getShopCenterIconRow( uint id ) const
{
	ShopCenterIconTable::const_iterator it = mShopCenterIconTable.find( id );
	if ( it == mShopCenterIconTable.end() )
		return NULL;

	return &it->second;
}
//-----------------------------------------------------------------------
const ShopCenterIconRow* LogicTableManager::getFitModeShopCenterIconRow( uint id ) const
{
	ShopCenterIconTable::const_iterator it = mFitModeShopCenterIconTable.find( id );
	if ( it == mFitModeShopCenterIconTable.end() )
		return NULL;

	return &it->second;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadShopCenterIconTable()
{
	XmlDataTable x;
	x.loadResource( OpenResource( "sys_shop_icon.xml" ) );

	const size_t colType		= x.getColumnIndex( "type" );
	const size_t colImagset		= x.getColumnIndex( "imageset" );
	const size_t colNormal		= x.getColumnIndex( "normal" );
	const size_t colPush		= x.getColumnIndex( "push" );
	const size_t colHover		= x.getColumnIndex( "hover" );
	const size_t colTooltip		= x.getColumnIndex( "txt" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		ShopCenterIconRow r;
		r.type		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType, true ) );
		r.imageset	= x.cell( iRow, colImagset, true );
		r.normal	= x.cell( iRow, colNormal, true );
		r.push		= x.cell( iRow, colPush, true );
		r.hover		= x.cell( iRow, colHover, true );
		r.tooltip	= GB2312ToUTF8(x.cell( iRow, colTooltip, true ).c_str());

		if ( mShopCenterIconTable.find( r.type ) != mShopCenterIconTable.end() )
			EQ_EXCEPT( "sys_shop_tplt.xml ID Repeat", "loadShopCenterTable" );

		if ( 0 == r.type )
			EQ_EXCEPT( "sys_shop_tplt.xml ID null", "loadShopCenterTable" );					

		mShopCenterIconTable.insert( std::make_pair( r.type, r ) );
	}	

	x.clear();
 	x.loadResource( OpenResource( "fit_shop_icon.xml" ) );
 	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
 	{		
 		ShopCenterIconRow r;
 		r.type		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType, true ) );
 		r.imageset	= x.cell( iRow, colImagset, true );
 		r.normal	= x.cell( iRow, colNormal, true );
 		r.push		= x.cell( iRow, colPush, true );
 		r.hover		= x.cell( iRow, colHover, true );
 		r.tooltip	= GB2312ToUTF8(x.cell( iRow, colTooltip, true ).c_str());
 
 		if ( mFitModeShopCenterIconTable.find( r.type ) != mFitModeShopCenterIconTable.end() )
 			EQ_EXCEPT( "sys_shop_tplt.xml ID Repeat", "loadShopCenterTable" );
 
 		if ( 0 == r.type )
 			EQ_EXCEPT( "sys_shop_tplt.xml ID null", "loadShopCenterTable" );					
 
 		mFitModeShopCenterIconTable.insert( std::make_pair( r.type, r ) );
 	}
}
//-----------------------------------------------------------------------
const ShopCenterRow* LogicTableManager::getNpcShopRow( uint id ) const
{
	ShopCenterTable::const_iterator it = mNpcShopTable.find( id );
	if ( it == mNpcShopTable.end() )
		return NULL;

	return &it->second;
}
//-----------------------------------------------------------------------
const ShopCenterRow* LogicTableManager::getNpcShopRowByItemID( uint itemid ) const
{
	ShopCenterTable::const_iterator it;
	for(it=mNpcShopTable.begin(); it!=mNpcShopTable.end(); it++)
	{
		if(it->second.item_id == itemid)
			return &it->second;
	}

	return NULL;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadNpcShopTable()
{
	XmlDataTable x;

	x.loadResource( OpenResource( "npc_shop_tplt.xml" ) );

	const size_t colGoodID		= x.getColumnIndex( "goods_id" );
	const size_t colItem		= x.getColumnIndex( "item_id" );
	const size_t colType		= x.getColumnIndex( "type" );
	const size_t colSex			= x.getColumnIndex( "sex" );
	const size_t colPrice		= x.getColumnIndex( "price" );
	const size_t colIntro		= x.getColumnIndex( "intro" );
	const size_t colSaleType	= x.getColumnIndex( "sale_type" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		ShopCenterRow r;
		r.good_id	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colGoodID ) );
		r.item_id	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colItem ) );
		r.type		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType, true ) );
		r.sex		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colSex, true ) );
		r.price		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colPrice, true ) );
		r.intro		= GB2312ToUTF8(x.cell( iRow, colIntro, true ).c_str());
		r.sale_type	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colSaleType ) );

		if ( mNpcShopTable.find( r.good_id ) != mNpcShopTable.end() )
			EQ_EXCEPT( "sys_shop_tplt.xml ID Repeat", "loadNpcShopTable" );

		if ( 0 == r.good_id )
			EQ_EXCEPT( "sys_shop_tplt.xml ID null", "loadNpcShopTable" );					

		mNpcShopTable.insert( std::make_pair( r.good_id, r ) );
	}
}
//-----------------------------------------------------------------------
void LogicTableManager::loadNpcShopMgrTable()
{
	XmlDataTable x;

	x.loadResource( OpenResource( "npc_shop_mgr.xml" ) );

	const size_t colID			= x.getColumnIndex( "id" );
	const size_t colSaleType	= x.getColumnIndex( "sale_type" );
	const size_t colPage		= x.getColumnIndex( "page" );
	const size_t colShopName	= x.getColumnIndex( "shop_name" );

	std::string str;
	std::vector<std::string> posArr; 
	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		NpcShopMgrRow r;
		r.id		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colID ) );
		r.shop_name	= GB2312ToUTF8(x.cell( iRow, colShopName, true ).c_str());

		str = x.cell( iRow, colSaleType, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()," );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				r.sale_type.push_back( Ogre::StringConverter::parseUnsignedInt(*it) );
			}
		}

		str = x.cell( iRow, colPage, true );
		Ogre::StringUtil::trim( str );
		if ( !str.empty() )
		{
			posArr = Ogre::StringUtil::split( str, " \r\n\t()," );
			for ( std::vector<std::string>::iterator it = posArr.begin(); it != posArr.end(); ++it )
			{
				int page = Ogre::StringConverter::parseUnsignedInt(*it);
				switch (page)
				{
				case 1:
					r.page1 = true;
					break;
				case 2:
					r.page2 = true;
					break;
				case 3:
					r.page3 = true;
					break;
				case 4:
					r.page4 = true;
					break;
				case 5:
					r.page5 = true;
					break;
				}
				if(r.default_page == 0)
					r.default_page = page;
			}
		}

		if ( mNpcShopMgrTable.find( r.id ) != mNpcShopMgrTable.end() )
			EQ_EXCEPT( "npc_shop_mgr.xml ID Repeat", "loadNpcShopMgrTable" );

		if ( 0 == r.id )
			EQ_EXCEPT( "npc_shop_mgr.xml ID null", "loadNpcShopMgrTable" );					

		mNpcShopMgrTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
const NpcShopMgrRow* LogicTableManager::getNpcShopMgrRow( uint id ) const
{
	NpcShopMgrTable::const_iterator it = mNpcShopMgrTable.find( id );
	if ( it == mNpcShopMgrTable.end() )
		return NULL;

	return &it->second;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadHouseSalesTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource( "house_transaction_tplt.xml" ) );

	const size_t colIndexId			= x.getColumnIndex( "id" );
	const size_t colName			= x.getColumnIndex( "name" );
	const size_t colType			= x.getColumnIndex("type");
	const size_t colRecomment		= x.getColumnIndex("recommend");
	const size_t colPayType			= x.getColumnIndex("pay_type");
	const size_t colLevel			= x.getColumnIndex( "level" );
	const size_t colPrice			= x.getColumnIndex( "price" );
	const size_t colFlower			= x.getColumnIndex("cost_flower");
	const size_t colNeedDeco		= x.getColumnIndex("need_decoration");
	const size_t colHouseId			= x.getColumnIndex( "house_id" );
	const size_t colDescript		= x.getColumnIndex( "intro" );

	for ( size_t iRow = 0; iRow < x.getRowCount(); ++iRow )
	{		
		HouseSalesRow r;
		r.indexId	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colIndexId ) );
		r.name		= GB2312ToUTF8(x.cell( iRow, colName, true ).c_str());
		r.type		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType ) );
		r.recommend	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colRecomment ) )!= 0;
		r.level		= Ogre::StringConverter::parseInt( x.cell( iRow, colLevel ) );
		r.payType	= Ogre::StringConverter::parseInt( x.cell( iRow, colPayType ) );
		r.price		= Ogre::StringConverter::parseInt( x.cell( iRow, colPrice ) );
		r.cost_flower = Ogre::StringConverter::parseInt( x.cell( iRow, colFlower ) );
		r.need_decoration = Ogre::StringConverter::parseInt( x.cell( iRow, colNeedDeco ) );
		r.houseId	= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colHouseId ) );
		r.descriptInfo = GB2312ToUTF8(x.cell( iRow, colDescript, true ).c_str());

		if ( mHouseSalesTable.find( r.indexId ) != mHouseSalesTable.end() )
			EQ_EXCEPT( "house_transaction_tplt.xml ID Repeat", "loadHouseSalesTable" );					

		mHouseSalesTable.insert( std::make_pair( r.indexId, r ) );
	}
}
//-----------------------------------------------------------------------
const HouseSalesRow* LogicTableManager::getHouseSalesRow(uint id) const
{
	HouseSalesTable::const_iterator iter = mHouseSalesTable.find(id);
	if(iter == mHouseSalesTable.end())
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadDoserviceTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("domestic_service_tplt.xml"));

	const size_t colId			= x.getColumnIndex( "id" );
	const size_t colDiscount	= x.getColumnIndex( "discount_rate" );
	const size_t colImage		= x.getColumnIndex( "image" );
	const size_t colName		= x.getColumnIndex( "name" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		DoServiceRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colId ) );
		r.addValue	    = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colDiscount ) );;
		r.image			= x.cell( iRow, colImage, true ).c_str();
		r.name			= GB2312ToUTF8(x.cell( iRow, colName, true ).c_str());

		if ( mDoserviceTable.find( r.id ) != mDoserviceTable.end() )
			EQ_EXCEPT( "domestic_service_tplt.xml ID Repeat", "loadDoserviceTable" );					

		mDoserviceTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
const DoServiceRow* LogicTableManager::getDoserviceRow(uint id) const
{
	DomesticServiceTable::const_iterator iter = mDoserviceTable.find(id);

	if (iter == mDoserviceTable.end())
		return NULL;
	
	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadWorksTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("work_tplt.xml"));

	const size_t colId			= x.getColumnIndex( "id" );
	const size_t grade			= x.getColumnIndex( "grade" );
	const size_t money			= x.getColumnIndex( "money" );
	const size_t clean			= x.getColumnIndex( "need_clean" );
	const size_t health			= x.getColumnIndex( "need_health" );
	const size_t charm			= x.getColumnIndex( "need_charm" );
	const size_t active			= x.getColumnIndex( "need_active" );
	const size_t healthState	= x.getColumnIndex( "need_disease" );
	const size_t propertyid		= x.getColumnIndex( "property_id" );
	const size_t colWorkImage	= x.getColumnIndex( "workImage" );
	const size_t colTypeImageSet= x.getColumnIndex( "typeImageSet" );
	const size_t colTypeImage	= x.getColumnIndex( "typeImage" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		WorkRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colId ) );
		r.workImage	    = x.cell( iRow, colWorkImage, true ).c_str();
		r.typeImageSet	= x.cell( iRow, colTypeImageSet, true ).c_str();
		r.typeImage		= x.cell( iRow, colTypeImage, true).c_str();
		r.grade			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, grade ) );
		r.money			= Ogre::StringConverter::parseInt( x.cell( iRow, money ) );
		r.clean			= Ogre::StringConverter::parseInt( x.cell( iRow, clean ) );
		r.health		= Ogre::StringConverter::parseInt( x.cell( iRow, health ) );
		r.charm			= Ogre::StringConverter::parseInt( x.cell( iRow, charm ) );
		r.active		= Ogre::StringConverter::parseInt( x.cell( iRow, active ) );
		r.healthState	= Ogre::StringConverter::parseInt( x.cell( iRow, healthState ) );
		r.propertyid	= Ogre::StringConverter::parseInt( x.cell( iRow, propertyid ) );

		if ( mWorkTable.find( r.id ) != mWorkTable.end() )
			EQ_EXCEPT( "work_tplt.xml ID Repeat", "loadWorksTable" );					

		mWorkTable.insert( std::make_pair( r.id, r ) );
	}

}
//-----------------------------------------------------------------------
const WorkRow * LogicTableManager::getWorkRow(uint id) const
{
	WorksTable::const_iterator iter = mWorkTable.find(id);

	if (iter == mWorkTable.end())
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadWorkEventTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("work_event_tplt.xml"));


	const size_t colId       = x.getColumnIndex( "id" );   
	const size_t colImage	 = x.getColumnIndex( "image" );
	
	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		WorkEventRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colId ) );
		r.image	    = x.cell( iRow, colImage, true ).c_str();

		if ( mWorkEventTable.find( r.id ) != mWorkEventTable.end() )
			EQ_EXCEPT( "work_event_tplt.xml ID Repeat", "loadWorksTable" );					

		mWorkEventTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
const WorkEventRow * LogicTableManager::getWorkEventRow(uint id) const
{
	WorkEventTable::const_iterator iter = mWorkEventTable.find(id);

	if(mWorkEventTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadTeleportTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("teleport_tplt.xml"));

	const size_t id				= x.getColumnIndex( "id" );   
	const size_t src_scene_id	= x.getColumnIndex( "src_scene_id" );
	const size_t src_x			= x.getColumnIndex( "src_x" );
	const size_t src_y			= x.getColumnIndex( "src_y" );
	const size_t src_z			= x.getColumnIndex( "src_z" );
	const size_t src_radius		= x.getColumnIndex( "src_radius" );

	Ogre::Vector3 tmp;
	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		TeleportRow r;
		r.id = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, id ) );
		r.src_scene_id = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, src_scene_id ) );
		r.src_x = Ogre::StringConverter::parseReal( x.cell( iRow, src_x ) );
		r.src_y = Ogre::StringConverter::parseReal( x.cell( iRow, src_y ) );
		r.src_z = Ogre::StringConverter::parseReal( x.cell( iRow, src_z ) );
		r.src_radius = Ogre::StringConverter::parseReal( x.cell( iRow, src_radius ) );
	
		if ( mTeleportTable.find( r.id ) != mTeleportTable.end() )
			EQ_EXCEPT( "teleport.xml ID Repeat", "loadteleportTable" );					

		mTeleportTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
void LogicTableManager::loadSceneTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("scene.xml"));


	const size_t colId				= x.getColumnIndex( "id" );   
	const size_t colName			= x.getColumnIndex( "name" );
	const size_t colType			= x.getColumnIndex( "type" );
	const size_t colFile			= x.getColumnIndex( "file" );
	const size_t colFarClipDistance	= x.getColumnIndex( "far_clip_distance" );
	const size_t pitchMin		= x.getColumnIndex( "pitchMin" );
	const size_t pitchMax		= x.getColumnIndex( "pitchMax" );
	const size_t yawMin			= x.getColumnIndex( "yawMin" );
	const size_t yawMax			= x.getColumnIndex( "yawMax" );
	const size_t zoomMin		= x.getColumnIndex( "zoomMin" );
	const size_t zoomMax		= x.getColumnIndex( "zoomMax" );
	const size_t lookatMin		= x.getColumnIndex( "lookatMin" );
	const size_t lookatMax		= x.getColumnIndex( "lookatMax" );
	const size_t voiceid		= x.getColumnIndex("voiceid");

	Ogre::Vector3 tmp;
	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		SceneRow r;
		r.id				= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colId ) );
		r.name				= x.cell( iRow, colName, true ).c_str();
		r.type				= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colType ) );
		r.file				= x.cell( iRow, colFile, true ).c_str();
		r.far_clip_distance	= Ogre::StringConverter::parseReal( x.cell( iRow, colFarClipDistance ) );
		if ( r.far_clip_distance == 0.0f )
			r.far_clip_distance = 50000.0f;

		r.pitchMin			= Ogre::StringConverter::parseInt( x.cell( iRow, pitchMin ) );
		r.pitchMax			= Ogre::StringConverter::parseInt( x.cell( iRow, pitchMax ) );
		r.yawMin			= Ogre::StringConverter::parseInt( x.cell( iRow, yawMin ) );
		r.yawMax			= Ogre::StringConverter::parseInt( x.cell( iRow, yawMax ) );
		r.zoomMin			= Ogre::StringConverter::parseInt( x.cell( iRow, zoomMin ) );
		r.zoomMax			= Ogre::StringConverter::parseInt( x.cell( iRow, zoomMax ) );
		r.lookatMin			= Ogre::StringConverter::parseVector3( x.cell( iRow, lookatMin ) );
		r.lookatMax			= Ogre::StringConverter::parseVector3( x.cell( iRow, lookatMax ) );
		r.voiceid			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, voiceid ) );

		if ( mSceneTable.find( r.id ) != mSceneTable.end() )
			EQ_EXCEPT( "common_scene.xml ID Repeat", "loadSceneTable" );					

		mSceneTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
const SceneRow* LogicTableManager::getSceneRow(uint id) const
{
	SceneTable::const_iterator it = mSceneTable.find(id);

	if(mSceneTable.end() == it)
		return NULL;

	return &(it->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadBornTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("born.xml"));

	const size_t colId		= x.getColumnIndex( "id" );   
	const size_t colPos		= x.getColumnIndex( "pos" );
	const size_t colDir		= x.getColumnIndex( "dir" );
	const size_t colLookat	= x.getColumnIndex( "lookat" );
	const size_t colPitch	= x.getColumnIndex( "pitch" );
	const size_t colYaw		= x.getColumnIndex( "yaw" );
	const size_t colZoom	= x.getColumnIndex( "zoom" );

	Ogre::Vector3 tmp;
	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		BornRow r;
		r.id				= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colId ) );
		r.pos				= Ogre::StringConverter::parseVector3( x.cell(iRow, colPos) );
		r.dir				= Ogre::StringConverter::parseUnsignedInt( x.cell(iRow, colDir) );
		r.lookat			= Ogre::StringConverter::parseVector3( x.cell(iRow, colLookat) );
		r.pitch				= Ogre::StringConverter::parseInt( x.cell( iRow, colPitch ) );
		r.yaw				= Ogre::StringConverter::parseInt( x.cell( iRow, colYaw ) );
		r.zoom				= Ogre::StringConverter::parseInt( x.cell( iRow, colZoom ) );

		if ( mBornTable.find( r.id ) != mBornTable.end() )
			EQ_EXCEPT( "born.xml ID Repeat", "loadBornTable" );					

		mBornTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
BornRow* LogicTableManager::getBornRow(uint id) const
{
	BornTable::const_iterator it = mBornTable.find(id);

	if(mBornTable.end() == it)
		return NULL;

	return (BornRow*)(&(it->second));
}
//-----------------------------------------------------------------------
const TeleportRow* LogicTableManager::getTeleportRow(uint id) const
{
	TeleportTable::const_iterator it = mTeleportTable.find(id);

	if(mTeleportTable.end() == it)
		return NULL;

	return &(it->second);
}

//-----------------------------------------------------------------------
void LogicTableManager::loadServicePriceTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("domestic_price_tplt.xml"));

	const size_t colLevel       = x.getColumnIndex( "house_level" );
	const size_t colPrice       = x.getColumnIndex( "money" );   

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		ServicePriceRow r;
		r.level		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colLevel ) );
		r.price	    = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colPrice ) );

		if ( mServicePriceTable.find( r.level ) != mServicePriceTable.end() )
			EQ_EXCEPT( "domestic_price_tplt.xml", "loadServicePriceTable" );					

		mServicePriceTable.insert( std::make_pair( r.level, r ) );
	}
};
//-----------------------------------------------------------------------
const ServicePriceRow* LogicTableManager::getServicePriceRow(uint level) const
{
	ServicePriceTable::const_iterator iter = mServicePriceTable.find(level);

	if(mServicePriceTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadLoadingTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("loading.xml"));

	const size_t colSet		= x.getColumnIndex( "set" );
	const size_t colImage	= x.getColumnIndex( "image" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		LoadingRow r;
		r.set		= x.cell( iRow, colSet );
		r.image	    = x.cell( iRow, colImage );				

		mLoadingTable.push_back(r);
	}

	XmlDataTable y;
	y.loadResource(OpenResource("loadingtxt.xml"));

	for(size_t iRow = 0; iRow < y.getRowCount(); ++iRow)
		mLoadingTipsTable.push_back( y.cell(iRow,y.getColumnIndex("txt")) );
}
//-----------------------------------------------------------------------
const LoadingRow* LogicTableManager::getRandomLoadingRow() const
{
	return &mLoadingTable[ (rand()%mLoadingTable.size()) ];
}
//-----------------------------------------------------------------------
std::string LogicTableManager::getRandomLoadingTips() const
{
	return mLoadingTipsTable[ (rand()%mLoadingTipsTable.size()) ];
}
//-----------------------------------------------------------------------
void LogicTableManager::loadOpenUITable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("openUI.xml"));

	const size_t name				= x.getColumnIndex( "name" );   
	const size_t relationSets		= x.getColumnIndex( "relationUI" );
	const size_t mutexSets			= x.getColumnIndex( "mutexUI" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		OpenUIRow row;

		row.name = x.cell( iRow, name, true ).c_str();
		row.relationUISets = x.cell( iRow, relationSets, true).c_str();
		row.mutexUISets = x.cell( iRow, mutexSets, true).c_str();

		if ( mOpenUITable.find( row.name ) != mOpenUITable.end() )
			EQ_EXCEPT( "OpenUI.xml name Repeat", "openUI" );					

		mOpenUITable.insert( std::make_pair( row.name, row ) );
	}
}
//-----------------------------------------------------------------------
const OpenUIRow * LogicTableManager::getOpenUIRow(const std::string winName) const
{
	OpenUITable::const_iterator iter = mOpenUITable.find(winName);

	if(mOpenUITable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
const GiftBoxRow * LogicTableManager::getGiftBoxRow(Ogre::uint id) const
{
	GiftBoxTable::const_iterator iter = mGiftBoxTable.find(id);

	if( mGiftBoxTable.end() == iter )
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadGiftBoxTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("gift_box_tplt.xml"));

	const size_t id				= x.getColumnIndex( "id" );   
	const size_t type			= x.getColumnIndex( "type" );
	const size_t eqGold			= x.getColumnIndex( "eq_coin" );
	const size_t gameGold		= x.getColumnIndex( "game_coin" );
	const size_t modelId		= x.getColumnIndex( "model_id" );
	const size_t name			= x.getColumnIndex( "name" );
	const size_t imageset		= x.getColumnIndex( "imageset" );
	const size_t image			= x.getColumnIndex( "image" );
	const size_t desc			= x.getColumnIndex( "desc" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		GiftBoxRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, id ) );
		row.type = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, type ) );
		row.eqGold = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, eqGold ) );
		row.gameGold = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, gameGold ) );
		row.modelId = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, modelId ) );
		row.name = GB2312ToUTF8(x.cell( iRow, name, true).c_str());
		row.imageset = GB2312ToUTF8(x.cell( iRow, imageset, true).c_str());
		row.image = GB2312ToUTF8(x.cell( iRow, image, true).c_str());
		row.desc = GB2312ToUTF8(x.cell( iRow, desc, true).c_str());

		if ( mGiftBoxTable.find( row.id ) != mGiftBoxTable.end() )
			EQ_EXCEPT( "gift_box_tplt.xml id Repeat", "loadGiftBoxTable" );					

		mGiftBoxTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
void LogicTableManager::loadGiftCardTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("gift_card_tplt.xml"));

	const size_t id				= x.getColumnIndex( "id" );   
	const size_t type			= x.getColumnIndex( "type" );
	const size_t eqGold			= x.getColumnIndex( "eq_coin" );
	const size_t gameGold		= x.getColumnIndex( "game_coin" );
	const size_t name			= x.getColumnIndex( "name" );
	const size_t imageset		= x.getColumnIndex( "imageset" );
	const size_t image			= x.getColumnIndex( "image" );
	const size_t desc			= x.getColumnIndex( "desc" );
	const size_t url			= x.getColumnIndex( "url" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		GiftCardRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, id ) );
		row.type = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, type ) );
		row.eqGold = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, eqGold ) );
		row.gameGold = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, gameGold ) );
		row.name = GB2312ToUTF8(x.cell( iRow, name, true).c_str());
		row.imageset = GB2312ToUTF8(x.cell( iRow, imageset, true).c_str());
		row.image = GB2312ToUTF8(x.cell( iRow, image, true).c_str());
		row.desc = GB2312ToUTF8(x.cell( iRow, desc, true).c_str());
		row.urlAddress = GB2312ToUTF8(x.cell( iRow, url, true).c_str());

		if ( mGiftCardTable.find( row.id ) != mGiftCardTable.end() )
			EQ_EXCEPT( "gift_card_tplt.xml id Repeat", "loadGiftCardTable" );					

		mGiftCardTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
const GiftCardRow * LogicTableManager::getGiftCardRow(uint id) const
{
	GiftCardTable::const_iterator iter = mGiftCardTable.find(id);

	if(mGiftCardTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadGotoTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("goto.xml"));

	const size_t id			= x.getColumnIndex( "id" );   
	const size_t page		= x.getColumnIndex( "page" );
	const size_t pos		= x.getColumnIndex( "pos" );
	const size_t image		= x.getColumnIndex( "image" );
	const size_t name		= x.getColumnIndex( "name" );
	const size_t target		= x.getColumnIndex( "target" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		GotoRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, id ) );
		row.page = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, page ) );
		row.pos = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, pos ) );
		row.image = x.cell(iRow, image);
		row.name = GB2312ToUTF8(x.cell( iRow, name, true).c_str());
		row.target = Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, target ) );

		if ( mGotoTable.find( row.id ) != mGotoTable.end() )
			EQ_EXCEPT( "goto.xml id Repeat", "loadGotoTable" );					

		mGotoTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
const GotoRow* LogicTableManager::getGotoRow( uint id ) const
{
	GotoTable::const_iterator iter = mGotoTable.find(id);

	if(mGotoTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
const GotoRow* LogicTableManager::getGotoRowByName(std::string name) const
{
	GotoTable::const_iterator iter = mGotoTable.begin();

	for (iter; mGotoTable.end()!=iter; ++iter)
	{
		if (iter->second.name == GB2312ToUTF8(name.c_str()))
		{
			return &(iter->second);
		}
	}

	return NULL;
}
//-----------------------------------------------------------------------
#define GOTO_USER_DATA_FILE "guide.user"
#define GOTO_USER_DATA_REC "Rec"
#define GOTO_USER_DATA_FAV "Fav"
std::string LogicTableManager::getGotoUserData( std::string acc, uint type )
{
	std::string str;
	std::string strFile = GetGameUserDir() + GOTO_USER_DATA_FILE;
	char szTemp[MAX_PATH+1];

	if(0 == type)
		str = GOTO_USER_DATA_REC;
	else
		str = GOTO_USER_DATA_FAV;

	GetPrivateProfileStringA(acc.c_str(), str.c_str(), "", szTemp, MAX_PATH, strFile.c_str());

	str = szTemp;

	return str;
}

void LogicTableManager::setGotoUserData( std::string acc, uint type, std::string data )
{
	std::string str;
	std::string strFile = GetGameUserDir() + GOTO_USER_DATA_FILE;

	if(0 == type)
		str = GOTO_USER_DATA_REC;
	else
		str = GOTO_USER_DATA_FAV;

	WritePrivateProfileStringA(acc.c_str(), str.c_str(), data.c_str(), strFile.c_str());
}
//-----------------------------------------------------------------------
void LogicTableManager::loadGoodsDescribeTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("goods_describe_table.xml"));

	const int id		= x.getColumnIndex("id");
	const size_t value	= x.getColumnIndex("value");

	for (size_t iRow=0; iRow<x.getRowCount(); ++iRow)
	{
		GoodsDescribeRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow, id));
		row.value = x.cell(iRow, value).c_str();

		if (mGoodsDescribeTable.find(row.id) != mGoodsDescribeTable.end())
			EQ_EXCEPT("goods_describe_table.xml id Repeat", "loadGoodsDescribeTable");					

		mGoodsDescribeTable.insert(std::make_pair(row.id, row));
	}
}
//-----------------------------------------------------------------------
const GoodsDescribeRow* LogicTableManager::getGoodsDescribeRow(uint id) const
{
	GoodsDescribeTable::const_iterator iter = mGoodsDescribeTable.find(id);
	if(mGoodsDescribeTable.end() == iter)
	{
		return NULL;
	}
	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadPlayerGuideTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("playerGuide.xml"));

	const size_t id			= x.getColumnIndex("id");
	const size_t num		= x.getColumnIndex("num");
	const size_t imageset	= x.getColumnIndex("imageset");
	const size_t image		= x.getColumnIndex("image");

	std::string temp_str;
	std::vector<std::string> temp_str_vec; 

	for(size_t iRow=0; iRow<x.getRowCount(); ++iRow)
	{
		PlayerGuideRow r;
		r.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow, id));
		// row,col
		temp_str = x.cell(iRow, num, true);
		Ogre::StringUtil::trim(temp_str);
		if (!temp_str.empty())
		{
			temp_str_vec = Ogre::StringUtil::split(temp_str, ",");
			if (2 != temp_str_vec.size())
			{
				EQ_EXCEPT("playerGuide.xml num error", "loadPlayerGuideTable");
			}
			r.row = atoi((char*)temp_str_vec[0].c_str());
			r.col = atoi((char*)temp_str_vec[1].c_str());
		}
		// imageset
		temp_str = x.cell(iRow, imageset, true);
		Ogre::StringUtil::trim(temp_str);
		if (!temp_str.empty())
		{
			temp_str_vec = Ogre::StringUtil::split(temp_str, ",");
			for (std::vector<std::string>::iterator it=temp_str_vec.begin(); temp_str_vec.end()!=it; ++it)
			{
				r.imageset.push_back(*it);
			}
		}
		// image
		temp_str = x.cell(iRow, image, true);
		Ogre::StringUtil::trim(temp_str);
		if (!temp_str.empty())
		{
			temp_str_vec = Ogre::StringUtil::split(temp_str, ",");
			for (std::vector<std::string>::iterator it=temp_str_vec.begin(); temp_str_vec.end()!=it; ++it)
			{
				r.image.push_back(*it);
			}
		}

		if (mPlayerGuideTable.find(r.id) != mPlayerGuideTable.end())
			EQ_EXCEPT("playerGuide.xml id Repeat", "loadPlayerGuideTable");					

		mPlayerGuideTable.insert(std::make_pair(r.id, r));
	}
}
//-----------------------------------------------------------------------
const PlayerGuideRow* LogicTableManager::getPlayerGuideRow(uint id) const
{
	PlayerGuideTable::const_iterator iter = mPlayerGuideTable.find(id);
	if(mPlayerGuideTable.end() == iter)
	{
		return NULL;
	}
	return &(iter->second);
}
//-----------------------------------------------------------------------
std::string LogicTableManager::getPlayerGuideImageset(uint id, uint i)
{
	const PlayerGuideRow* row = getPlayerGuideRow(id);
	if (NULL == row)
	{
		return "";
	}
	if ((i<0) || ((size_t)i>=row->imageset.size()))
	{
		return "";
	}
	return row->imageset[i];
}
//-----------------------------------------------------------------------
std::string LogicTableManager::getPlayerGuideImage(uint id, uint i)
{
	const PlayerGuideRow* row = getPlayerGuideRow(id);
	if (NULL == row)
	{
		return "";
	}
	if ((i<0) || ((size_t)i>=row->image.size()))
	{
		return "";
	}
	return row->image[i];
}
//-----------------------------------------------------------------------
void LogicTableManager::loadNoviceTeachStepTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("novice_teach_steps_tplt.xml"));

	const size_t step		= x.getColumnIndex("step");
	const size_t content	= x.getColumnIndex("content");
	const size_t pre_idx	= x.getColumnIndex("preBtnStr");
	const size_t next_idx	= x.getColumnIndex("nextBtnStr");

	for (size_t iRow=0; iRow<x.getRowCount(); ++iRow)
	{
		NoviceTeachStepRow row;

		row.step	= Ogre::StringConverter::parseUnsignedInt(x.cell(iRow, step));
		row.content	= x.cell(iRow, content).c_str();
		row.preBtnstr = x.cell(iRow,pre_idx).c_str();
		row.nextBtnstr = x.cell(iRow,next_idx).c_str();

		if ( mNoviceTeachStepTable.find( row.step ) != mNoviceTeachStepTable.end() )
			EQ_EXCEPT( "novice_teach_steps_tplt.xml step Repeat", "loadNoviceTeachStepTable" );

		mNoviceTeachStepTable.insert(std::make_pair(row.step, row));
	}
}
//-----------------------------------------------------------------------
const NoviceTeachStepRow* LogicTableManager::getNoviceTeachStepRow(uint step) const
{
	NoviceTeachStepTable::const_iterator iter = mNoviceTeachStepTable.find(step);
	if (mNoviceTeachStepTable.end() == iter)
	{
		return NULL;
	}
	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadMutexParentIdxTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("UIWindowIdx.xml"));

	const size_t id			= x.getColumnIndex( "id" );   
	const size_t name	    = x.getColumnIndex( "name" );
	const size_t parent		= x.getColumnIndex( "parent" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		MutexParentIdxRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,id));
		row.parent = Ogre::StringConverter::parseInt(x.cell(iRow,parent));
		row.name = GB2312ToUTF8(x.cell(iRow, name).c_str());

		if ( mMutexParentIdxTable.find( row.id ) != mMutexParentIdxTable.end() )
			EQ_EXCEPT( "parentIdx.xml name Repeat", "mMutexParentIdxTable" );					

		mMutexParentIdxTable.insert( std::make_pair( row.id, row ) );
	}

}
//-----------------------------------------------------------------------
const MutexParentIdxRow* LogicTableManager::getMutexParentIdxRow( uint id ) const
{
	MutexParentIdxTable::const_iterator iter = mMutexParentIdxTable.find(id);

	if(mMutexParentIdxTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}

const MutexParentIdxRow* LogicTableManager::getMutexParentIdxRowN( const std::string name ) const
{
	for(MutexParentIdxTable::const_iterator iter = mMutexParentIdxTable.begin(); iter != mMutexParentIdxTable.end(); ++iter)
	{
		if(iter->second.name == name)
		{
			return &(iter->second);
		}
	}

	return NULL;

}
//-----------------------------------------------------------------------
void LogicTableManager::loadMutexUITable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("openUI.xml"));

	const size_t name	    = x.getColumnIndex( "name" );
	const size_t relation	= x.getColumnIndex( "relationUI" );
	const size_t mutex		= x.getColumnIndex( "mutexUI" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		MutexUIRow row;

		row.name = GB2312ToUTF8(x.cell(iRow, name).c_str());
		row.relation = GB2312ToUTF8(x.cell(iRow, relation).c_str());
		row.mutex = GB2312ToUTF8(x.cell(iRow, mutex).c_str());

		if ( mMutexUITable.find( row.name ) != mMutexUITable.end() )
			EQ_EXCEPT( "openUI.xml name Repeat", "mMutexUITable" );					

		mMutexUITable.insert( std::make_pair( row.name, row ) );
	}
}
//-----------------------------------------------------------------------
const MutexUIRow* LogicTableManager::getMutexUIRow( const std::string name ) const
{
	MutexUITable::const_iterator iter = mMutexUITable.find(name);

	if(mMutexUITable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadMagicBoxTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("magic_box.xml"));

	const size_t id	    = x.getColumnIndex( "id" );
	const size_t modelId	= x.getColumnIndex( "modelId" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		MagicBoxRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,id));
		row.modelId = Ogre::StringConverter::parseInt(x.cell(iRow,modelId));

		if ( mMagicBoxTable.find( row.id ) != mMagicBoxTable.end() )
			EQ_EXCEPT( "magic_box.xml name Repeat", "mMagicBoxTable" );					

		mMagicBoxTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
const MagicBoxRow* LogicTableManager::getMagicBoxRow( uint id ) const
{
	MagicBoxTable::const_iterator iter = mMagicBoxTable.find(id);

	if(mMagicBoxTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);

}
//-----------------------------------------------------------------------
void LogicTableManager::loadGarbageTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("garbage.xml"));

	const size_t id	    = x.getColumnIndex( "id" );
	const size_t modelId	= x.getColumnIndex( "modelId" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		GarbageRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,id));
		row.modelId = Ogre::StringConverter::parseInt(x.cell(iRow,modelId));

		if ( mGarbageTable.find( row.id ) != mGarbageTable.end() )
			EQ_EXCEPT( "garbage.xml name Repeat", "mGarbageTable" );					

		mGarbageTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
const GarbageRow* LogicTableManager::getGarbageRow( uint id ) const
{
	GarbageTable::const_iterator iter = mGarbageTable.find(id);

	if(mGarbageTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadVoiceTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("voices.xml"));

	const size_t id	    = x.getColumnIndex( "id" );
	const size_t file	= x.getColumnIndex( "file" );
	const size_t cls	= x.getColumnIndex( "class" );
	const size_t ltype  = x.getColumnIndex( "ltype" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		VoiceRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,id));
		row.cls = Ogre::StringConverter::parseInt(x.cell(iRow,cls));
		row.ltype = Ogre::StringConverter::parseInt(x.cell(iRow,ltype));
		row.file = "../Media/Sound/" + GB2312ToUTF8(x.cell(iRow, file).c_str());

		if ( mVoiceTable.find( row.id ) != mVoiceTable.end() )
			EQ_EXCEPT( "voices.xml name Repeat", "mVoiceTable" );					

		mVoiceTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
const VoiceRow* LogicTableManager::getVoiceRow( uint id ) const
{
	VoiceTable::const_iterator iter = mVoiceTable.find(id);

	if(mVoiceTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadUIVoiceTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("ui_sound.xml"));

	const size_t id			= x.getColumnIndex( "id" );
	const size_t hovervid	= x.getColumnIndex( "hover_vid" );
	const size_t clickvid	= x.getColumnIndex( "click_vid" );
	const size_t name		= x.getColumnIndex( "ui_name" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		UIVoiceRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,id));
		row.hovervid = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,hovervid));
		row.clickvid = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,clickvid));
		row.name = GB2312ToUTF8(x.cell(iRow, name).c_str());

		if ( mUIVoiceTable.find( row.name ) != mUIVoiceTable.end() )
			EQ_EXCEPT( "ui_sound.xml name Repeat", "mUIVoiceTable" );					

		mUIVoiceTable.insert( std::make_pair( row.name, row ) );
	}
	
	mUIVoiceItr = mUIVoiceTable.end();
}
//-----------------------------------------------------------------------
const UIVoiceRow* LogicTableManager::getUIVoiceRow( const std::string name ) const
{
	UIVoiceTable::const_iterator iter = mUIVoiceTable.find(name);

	if(mUIVoiceTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
const UIVoiceRow* LogicTableManager::getUIVoiceRowByIdx( uint idx ) const
{
	UIVoiceTable::const_iterator iter = mUIVoiceTable.begin();

	while(idx)
	{
		if(mUIVoiceTable.end() == iter)
			return NULL;

		iter ++;
		idx --;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::getUIVoiceRowByIdxStart()
{
	mUIVoiceItr = mUIVoiceTable.begin();

}
//-----------------------------------------------------------------------
const UIVoiceRow* LogicTableManager::getUIVoiceRowByIdxNext()
{
	if(mUIVoiceTable.end() == mUIVoiceItr)
		return NULL;

	const UIVoiceRow *row = &(mUIVoiceItr->second);

	mUIVoiceItr ++;

	return row;
}
//-----------------------------------------------------------------------
void LogicTableManager::loadSpecialVoiceTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("special_sound.xml"));

	const size_t id	    = x.getColumnIndex( "id" );
	const size_t voiceid= x.getColumnIndex( "voiceid" );
	const size_t name   = x.getColumnIndex( "name" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		SpecialVoiceRow row;

		row.id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,id));
		row.voiceid = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow,voiceid));
		row.name = GB2312ToUTF8(x.cell(iRow, name).c_str());

		if ( mSpecialVoiceTable.find( row.id ) != mSpecialVoiceTable.end() )
			EQ_EXCEPT( "special_sound.xml name Repeat", "mSpecialVoiceTable" );					

		mSpecialVoiceTable.insert( std::make_pair( row.id, row ) );
	}
}
//-----------------------------------------------------------------------
const SpecialVoiceRow* LogicTableManager::getSpecialVoiceRow( uint id ) const
{
	SpecialVoiceTable::const_iterator iter = mSpecialVoiceTable.find(id);

	if(mSpecialVoiceTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}

const PlayerPropertyRow* LogicTableManager::getPlayerPropertyRow( uint id ) const
{
	PlayerPropertyTable::const_iterator iter = mPlayerPropertyTable.find(id);

	if(mPlayerPropertyTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}

void LogicTableManager::loadPlayerPropertyTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("player_property_tplt.xml"));

	const size_t colId			= x.getColumnIndex( "property_id" );
	const size_t clean			= x.getColumnIndex( "player_clean" );
	const size_t health			= x.getColumnIndex( "player_health" );
	const size_t charm			= x.getColumnIndex( "player_charm" );
	const size_t active			= x.getColumnIndex( "active_value" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		PlayerPropertyRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colId ) );
		r.clean			= Ogre::StringConverter::parseInt( x.cell( iRow, clean ) );
		r.health		= Ogre::StringConverter::parseInt( x.cell( iRow, health ) );
		r.charm			= Ogre::StringConverter::parseInt( x.cell( iRow, charm ) );
		r.active		= Ogre::StringConverter::parseInt( x.cell( iRow, active ) );

		if ( mPlayerPropertyTable.find( r.id ) != mPlayerPropertyTable.end() )
			EQ_EXCEPT( "player_property_tplt.xml ID Repeat", "loadPlayerPropertyTable" );					

		mPlayerPropertyTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
void LogicTableManager::loadDiseaseTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("player_disease.xml"));

	const size_t colHealth		= x.getColumnIndex( "health" );
	const size_t colParticle	= x.getColumnIndex( "particle" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		DiseaseRow r;
		r.health		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colHealth ) );
		r.particle		= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, colParticle ) );

		if ( mDiseaseTable.find( r.health ) != mDiseaseTable.end() )
			EQ_EXCEPT( "player_disease.xml ID Repeat", "loadDiseaseTable" );					

		mDiseaseTable.insert( std::make_pair( r.health, r ) );
	}
}

const DiseaseRow* LogicTableManager::getDiseaseRow(uint id) const
{
	DiseaseTable::const_iterator it = mDiseaseTable.find(id);
	if(mDiseaseTable.end() == it)
		return NULL;

	return &(it->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadDiseaseSpecialEventTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("player_disease_special_event_tplt.xml"));

	const size_t id				= x.getColumnIndex( "special_event_id" );
	const size_t pid			= x.getColumnIndex( "property_id" );
	const size_t images			= x.getColumnIndex( "image" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		DiseaseSpecialEventRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, id ) );
		r.pid			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, pid ) );
		r.images		= x.cell( iRow, images ).c_str();

		if ( mDiseaseSpecialEventTable.find( r.id ) != mDiseaseSpecialEventTable.end() )
			EQ_EXCEPT( "player_disease_special_event_tplt.xml ID Repeat", "loadDiseaseSpecialEventTable" );					

		mDiseaseSpecialEventTable.insert( std::make_pair( r.id, r ) );
	}
}

const DiseaseSpecialEventRow* LogicTableManager::getDiseaseSpecialEventRow(uint id) const
{
	DiseaseSpecialEventTable::const_iterator iter = mDiseaseSpecialEventTable.find(id);

	if(mDiseaseSpecialEventTable.end() == iter)
	{
		return NULL;
	}

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadChangeWallFloorMaterialTable()
{
	XmlDataTable x;
	x.loadResource(OpenResource("change_wall_floor_mat.xml"));

	const size_t id				= x.getColumnIndex( "id" );
	const size_t type			= x.getColumnIndex( "type" );
	const size_t name			= x.getColumnIndex( "name" );
	const size_t material		= x.getColumnIndex( "material" );

	for(size_t iRow = 0; iRow < x.getRowCount(); ++iRow)
	{
		ChangeWallFloorMaterialRow r;
		r.id			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, id ) );
		r.type			= Ogre::StringConverter::parseUnsignedInt( x.cell( iRow, type ) );
		r.name			= x.cell( iRow, name ).c_str();
		r.material		= x.cell( iRow, material ).c_str();

		if ( mChangeWallFloorMaterialTable.find( r.id ) != mChangeWallFloorMaterialTable.end() )
			EQ_EXCEPT( "change_wall_floor_mat.xml ID Repeat", "loadmChangeWallFloorMaterialTable" );					

		mChangeWallFloorMaterialTable.insert( std::make_pair( r.id, r ) );
	}
}
//-----------------------------------------------------------------------
const ChangeWallFloorMaterialRow* LogicTableManager::getChangeWallFloorMaterialRow(uint id) const
{
	ChangeWallFloorMaterialTable::const_iterator iter = mChangeWallFloorMaterialTable.find(id);

	if (mChangeWallFloorMaterialTable.end() == iter)
	{
		return NULL;
	}
	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableManager::loadMiniMapTalbe()
{
	XmlDataTable x;
	x.loadResource(OpenResource("mini_map.xml"));

	const size_t scene_id	= x.getColumnIndex("scene_id");
	const size_t num		= x.getColumnIndex("num");
	const size_t imageset	= x.getColumnIndex("imageset");
	const size_t image		= x.getColumnIndex("image");

	std::string temp_str;
	std::vector<std::string> temp_str_vec; 

	for(size_t iRow=0; iRow<x.getRowCount(); ++iRow)
	{
		MiniMapRow r;
		r.scene_id = Ogre::StringConverter::parseUnsignedInt(x.cell(iRow, scene_id));
		// row,col
		temp_str = x.cell(iRow, num, true);
		Ogre::StringUtil::trim(temp_str);
		if (!temp_str.empty())
		{
			temp_str_vec = Ogre::StringUtil::split(temp_str, ",");
			if (2 != temp_str_vec.size())
			{
				EQ_EXCEPT("mini_map.xml num error", "loadMiniMapTalbe");
			}
			r.row = atoi((char*)temp_str_vec[0].c_str());
			r.col = atoi((char*)temp_str_vec[1].c_str());
		}
		// imageset
		temp_str = x.cell(iRow, imageset, true);
		Ogre::StringUtil::trim(temp_str);
		if (!temp_str.empty())
		{
			temp_str_vec = Ogre::StringUtil::split(temp_str, ",");
			for (std::vector<std::string>::iterator it=temp_str_vec.begin(); temp_str_vec.end()!=it; ++it)
			{
				r.imageset.push_back(*it);
			}
		}
		// image
		temp_str = x.cell(iRow, image, true);
		Ogre::StringUtil::trim(temp_str);
		if (!temp_str.empty())
		{
			temp_str_vec = Ogre::StringUtil::split(temp_str, ",");
			for (std::vector<std::string>::iterator it=temp_str_vec.begin(); temp_str_vec.end()!=it; ++it)
			{
				r.image.push_back(*it);
			}
		}

		if (mMiniMapTable.find(r.scene_id) != mMiniMapTable.end())
			EQ_EXCEPT("mini_map.xml id Repeat", "loadMiniMapTalbe");					

		mMiniMapTable.insert(std::make_pair(r.scene_id, r));
	}
}
//-----------------------------------------------------------------------
const MiniMapRow* LogicTableManager::getMiniMapRow(uint scene_id) const
{
	MiniMapTable::const_iterator iter = mMiniMapTable.find(scene_id);
	if(mMiniMapTable.end() == iter)
	{
		return NULL;
	}
	return &(iter->second);
}
//-----------------------------------------------------------------------
std::string LogicTableManager::getMiniMapImageset(uint scene_id, uint i)
{
	const MiniMapRow* row = getMiniMapRow(scene_id);
	if (NULL == row)
	{
		return "";
	}
	if ((i<0) || ((size_t)i>=row->imageset.size()))
	{
		return "";
	}
	return row->imageset[i];
}
//-----------------------------------------------------------------------
std::string LogicTableManager::getMiniMapImage(uint scene_id, uint i)
{
	const MiniMapRow* row = getMiniMapRow(scene_id);
	if (NULL == row)
	{
		return "";
	}
	if ((i<0) || ((size_t)i>=row->image.size()))
	{
		return "";
	}
	return row->image[i];
}
//-----------------------------------------------------------------------