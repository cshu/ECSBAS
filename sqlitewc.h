//don't add #pragma once

#define SQLITE_TRANSIENT_STATIC_BEF_FIN SQLITE_TRANSIENT//not sure if SQLITE_TRANSIENT can be used when buffer is const before finalization, so use this macro, may switch to SQLITE_STATIC for better performance

#define BOOKMARK_LEN_PREFIX_LIMIT 0x80
template<class Ts>
auto makefilenmfromurl(const Ts *u){
	auto tbr{regex_replace(u,regex("[^-_.0-9A-Za-z]+"),"_")};
	if(tbr.size()>BOOKMARK_LEN_PREFIX_LIMIT)tbr.resize(BOOKMARK_LEN_PREFIX_LIMIT);
	return tbr;
}
#include <sstream>
template<class T>
auto ensure_bm_filenm_is_unique(T &tbr){
	unique_ptr<integer_good_randomness<int>> igr;
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from sp_urlbookmar where lower(f)=lower(?)"));)
	for(;;){
		auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, tbr.c_str(),tbr.size(), SQLITE_TRANSIENT_STATIC_BEF_FIN);
		if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
		ib=sqlite3_step(sstmt.s.pstmt);
		switch(ib){
		case SQLITE_ROW:
			sqlite3_reset(sstmt.s.pstmt);
			if(igr){
				std::stringstream stream;
				stream << std::hex << igr->gener();
				tbr.replace(tbr.begin()+tbr.rfind('_'),tbr.end(),'_'+stream.str());
			}else{
				igr.reset(new integer_good_randomness<int>);
				std::stringstream stream;
				stream << std::hex << igr->gener();
				tbr+='_'+stream.str();
			}
			continue;
		case SQLITE_DONE:
			break;
		default:{LOG_Is(ib) throw 0;}
		}
		break;
	}
	CATCH_SET_SUE_THROWs(;)
}

template<class Ts>
auto makeuniquefilenmfromurl(const Ts *u){
	auto tbr=makefilenmfromurl(u);
	ensure_bm_filenm_is_unique(tbr);
	return tbr;
}

template<class Tvb,class Tsens>
void selectprops(Tvb &vb,Tsens sens){
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select in_singleargv.v,d from in_singleargp join in_singleargv on in_singleargp.v=in_singleargv.v where in_singleargp.s=?"));)
	auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, sens);
	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
	for(;;){
		ib=sqlite3_step(sstmt.s.pstmt);
		switch(ib){
		case SQLITE_ROW:{
			auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);
			vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
			ib=sqlite3_column_bytes(sstmt.s.pstmt,1);
			vb.resize(vb.size()+4);
			write_u32le(vb.data()+vb.size()-4,ib);
			auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);
			vb.insert(vb.end(),pcc,pcc+ib);
			continue;
		}
		case SQLITE_DONE:
			break;
		default:{LOG_Is(ib) throw 0;}
		}
		break;
	}
	CATCH_SET_SUE_THROWs(;)
}

#define DELETEALLSINGLEARGPROPSOFSENSs(sens) {\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_singleargp where s=?"));)\
	auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sens);\
	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
	ib=sqlite3_step(sstmt.s.pstmt);\
	if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}\
	CATCH_SET_SUE_THROWs(;)\
}

#define RECEIVESINGLEARGPROPSCHKANDINSERTs(offset,sens,stmtafterchk){\
	INIT_TRYs(dtor_delete_from_tmptab_i0 dtor_delete_from_tmptab_i0_;)\
	TEMPRECEIVESINGLEARGPROPSWITHTMPTABANDCHKs(offset)\
	{stmtafterchk}\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_singleargp where s=? and v not in tmptab_i0"));)\
	auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sens);\
	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
	ib=sqlite3_step(sstmt.s.pstmt);\
	if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_singleargp select z,?1 from tmptab_i0 where z not in(select v from in_singleargp where s=?1)"));)\
	auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sens);\
	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
	ib=sqlite3_step(sstmt.s.pstmt);\
	if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}\
	CATCH_SET_SUE_THROWs(;)\
	CATCH_SET_SUE_THROWs(;)\
}

#define TEMPSINGLEARGPROPSWITHTMPTABANDCHKs {\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from tmptab_i0 where z not in(select v from in_singleargv)"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Failed: Verb not found (might be already deleted)."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from tmptab_i0 group by z having count(*)>1"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Failed: Tautology."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(select z from tmptab_i0 union select c from in_entailment join a_a on a=z)select 0 from a_a join in_mutexgroup on v=z group by g having count(*)>1"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Failed: Oxymoron."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_b(y)as(select c from in_entailment join tmptab_i0 on a=z union all select c from in_entailment join a_b on a=y)select 0 from tmptab_i0 join a_b on z=y"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Failed: Tautology."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
}

//template<class T1,class T2>
//int tempReceiveSingleArgPropsWithTmpTabAndChk(T1 rb,T2 rl){
#define TEMPRECEIVESINGLEARGPROPSWITHTMPTABANDCHKs(offset) {\
	TEMPRECEIVESAIDWITHTMPTAB(offset)\
	TEMPSINGLEARGPROPSWITHTMPTABANDCHKs\
}

#define TEMPRECEIVESAIDWITHTMPTAB(offset) {\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into tmptab_i0 values(?)"));)\
	for(decltype(rl) offs=(offset);;){\
		sqlite3_int64 id;\
		memcpy(&id,rb+offs,sizeof id);\
		auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);\
		if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
		ib=sqlite3_step(sstmt.s.pstmt);\
		if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}\
		offs+=8;\
		if(offs==rl) break;\
		sqlite3_reset(sstmt.s.pstmt);\
	}\
	CATCH_SET_SUE_THROWs(;)\
}

#define TEMPRECEIVESINGLEARGVERBSWITHTMPTABANDCHKs(offset) {\
	TEMPRECEIVESAIDWITHTMPTAB(offset)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from tmptab_i0 where z not in(select v from in_singleargv)limit 1"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Failed: Verb not found (might be already deleted)."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from tmptab_i0 group by z having count(*)>1 limit 1"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Invalid Request: Verb appears more than once in group."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(select z from tmptab_i0 union select a from in_entailment join a_a on c=z)select s from in_singleargp where v in a_a group by s having count(*)>1"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Invalid States: two or more verbs in the group (including verbs beneath) are already attached to some record."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(y,z)as(select z,z from tmptab_i0 union select y,a from in_entailment join a_a on c=z)select z from a_a group by z having count(*)>1"));)\
	auto ib=sqlite3_step(sstmt.s.pstmt);\
	switch(ib){\
	case SQLITE_ROW:\
		setresnotifyfe(LITERAL_COMMA_LEN("\1Oxymoron: verbs in the group have some kind of association (a verb entailing another, or two verbs entailed by some verb)."));\
		goto begin_db_l;\
	case SQLITE_DONE:\
		break;\
	default:{LOG_Is(ib) throw 0;}\
	}\
	CATCH_SET_SUE_THROWs(;)\
}

CPPRS_COMMON_SP sqlite3_int64 ret_avail_globalid(void){
	auto vr=availid;
	++availid;
	return vr;
}

CPPRS_COMMON_SP void update_avail_sense_id(void){
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update memoizedcfg_i set v=? where k='AVAIL_SENSE_ID'"));)
	auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, availid);
	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
	ib=sqlite3_step(sstmt.s.pstmt);
	if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
	CATCH_SET_SUE_THROWs(;)
}

CPPRS_COMMON_SP void begin_tr(void){
	char *zErrMsg = nullptr;
	auto ib=sqlite3_exec(databaseA, "BEGIN EXCLUSIVE", NULL, NULL, &zErrMsg);
	if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
}

CPPRS_COMMON_SP void end_tr(void){
	char *zErrMsg = nullptr;
	auto ib=sqlite3_exec(databaseA, "COMMIT", NULL, NULL, &zErrMsg);
	if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
}

CPPRS_COMMON_SP void rb_tr(void){
	char *zErrMsg = nullptr;
	auto ib=sqlite3_exec(databaseA, "ROLLBACK", NULL, NULL, &zErrMsg);
	if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
}

/*
** must use INIT_TRYs and CATCH_SET_SUE_THROWs
*/
struct dtor_sqlite_close{
        virtual ~dtor_sqlite_close()noexcept(false){
		AUTO_COPY_OF_SUE_CLEAR_SUE_TRYs
		auto ib=sqlite3_close(databaseA);
		if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
		CATCH_IF_CHECK_COPY_OF_SUE_THROWs
	}
};

/*
** must use INIT_TRYs and CATCH_SET_SUE_THROWs
*/
struct dtor_sqlite_commit{
	bool begun=false;
	virtual ~dtor_sqlite_commit()noexcept(false){
		AUTO_COPY_OF_SUE_CLEAR_SUE_TRYs
		if(!begun)return;
		end_tr();
		CATCH_IF_CHECK_COPY_OF_SUE_THROWs
	}
};

/*
** must use INIT_TRYs and CATCH_SET_SUE_THROWs
*/
struct dtor_update_avail_sense_id{
	virtual ~dtor_update_avail_sense_id()noexcept(false){
		AUTO_COPY_OF_SUE_CLEAR_SUE_TRYs
		if(!availid)return;
		update_avail_sense_id();
		CATCH_IF_CHECK_COPY_OF_SUE_THROWs
	}
};

/*
** must use INIT_TRYs and CATCH_SET_SUE_THROWs
*/
struct dtor_delete_from_tmptab_b0{
	virtual ~dtor_delete_from_tmptab_b0()noexcept(false){
		AUTO_COPY_OF_SUE_CLEAR_SUE_TRYs
		char *zErrMsg=nullptr;
		auto ib=sqlite3_exec(databaseA,"delete from tmptab_b0", NULL, NULL, &zErrMsg);
		if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
		CATCH_IF_CHECK_COPY_OF_SUE_THROWs
	}
};
/*
** must use INIT_TRYs and CATCH_SET_SUE_THROWs
*/
struct dtor_delete_from_tmptab_i0{
	virtual ~dtor_delete_from_tmptab_i0()noexcept(false){
		AUTO_COPY_OF_SUE_CLEAR_SUE_TRYs
		char *zErrMsg=nullptr;
		auto ib=sqlite3_exec(databaseA,"delete from tmptab_i0", NULL, NULL, &zErrMsg);
		if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
		CATCH_IF_CHECK_COPY_OF_SUE_THROWs
	}
};
/*
** must use INIT_TRYs and CATCH_SET_SUE_THROWs
*/
struct dtor_delete_from_tmptab_ii0{
	virtual ~dtor_delete_from_tmptab_ii0()noexcept(false){
		AUTO_COPY_OF_SUE_CLEAR_SUE_TRYs
		char *zErrMsg=nullptr;
		auto ib=sqlite3_exec(databaseA,"delete from tmptab_ii0", NULL, NULL, &zErrMsg);
		if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
		CATCH_IF_CHECK_COPY_OF_SUE_THROWs
	}
};
