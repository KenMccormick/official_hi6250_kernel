/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : dmac_11w.h
  版 本 号   : 初稿
  作    者   : z00273164
  生成日期   : 2014年4月27日
  最近修改   :
  功能描述   : dmac_11w.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年4月27日
    作    者   : z00273164
    修改内容   : 创建文件

******************************************************************************/

#ifndef __DMAC_11W_H__
#define __DMAC_11W_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_types.h"
#include "dmac_vap.h"



/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

extern oal_uint32 dmac_bip_crypto(dmac_vap_stru *pst_dmac_vap,
                                           oal_netbuf_stru *pst_netbuf_mgmt,
                                           wlan_security_txop_params_stru  *pst_security,
                                           oal_uint16 *pus_len);
extern oal_uint32 dmac_11w_rx_filter(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru  *pst_netbuf);
extern oal_void dmac_11w_set_protectframe(dmac_vap_stru  *pst_dmac_vap,
                                            wlan_security_txop_params_stru *pst_security,
                                            oal_netbuf_stru *pst_netbuf);
extern oal_uint32 dmac_11w_get_pmf_cap(mac_vap_stru *pst_mac_vap, wlan_pmf_cap_status_uint8 *pen_pmf_cap);

extern oal_void dmac_11w_update_users_status(dmac_vap_stru  *pst_dmac_vap, mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_user_pmf);

#endif /* #if(_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT) */




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_11w.h */
