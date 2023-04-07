#include "iec103_handler.h"

#include <stdio.h>

#include "filter.h"
#include "print.h"

std::string CP32Time2aToString(const npacket::iec103::CP32Time2a& tm)
{
    char buf[128] = {0};
    sprintf(buf, "%02d:%02d %d(ms) summer[%d]", tm.hour, tm.minute, tm.millisecond, tm.summerTime);
    return buf;
}

std::string CP56Time2aToString(const npacket::iec103::CP56Time2a& tm)
{
    char buf[128] = {0};
    sprintf(buf, "%04d-%02d-%02d %02d:%02d %d(ms) wday[%d] summer[%d]", tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.millisecond,
            tm.wday, tm.summerTime);
    return buf;
}

/**
 * @brief 打印IEC103固定帧
 */
void printIEC103FixedFrame(const std::shared_ptr<npacket::iec103::FixedFrame>& frame)
{
    printf("            ----- IEC103 [fixed frame] -----\n");
    printf("            PRM: %d(%s)\n", frame->prm, 1 == frame->prm ? "mater to slave" : "slave to master");
    printf("            %s: %d\n", 1 == frame->prm ? "FCB" : "ACD", frame->fcb_acd);
    printf("            %s: %d\n", 1 == frame->prm ? "FCV" : "DFC", frame->fcv_dfc);
    printf("            FUNC: %d\n", frame->func);
    printf("            ADDR: %d\n", frame->addr);
}

/**
 * @brief 打印IEC103可变帧
 */
void printIEC103VariableFrame(const std::shared_ptr<npacket::iec103::VariableFrame>& frame)
{
    printf("            ----- IEC103 [variable frame] -----\n");
    printf("            PRM: %d(%s)\n", frame->prm, 1 == frame->prm ? "mater to slave" : "slave to master");
    printf("            %s: %d\n", 1 == frame->prm ? "FCB" : "ACD", frame->fcb_acd);
    printf("            %s: %d\n", 1 == frame->prm ? "FCV" : "DFC", frame->fcv_dfc);
    printf("            FUNC: %d\n", frame->func);
    printf("            ADDR: %d\n", frame->addr);
    if (!frame->asdu)
    {
        return;
    }
    printf("            ASDU%d:\n", frame->asdu->identify.type);
    printf("                  type: %d\n", frame->asdu->identify.type);
    printf("                  vsq: continuous[%d], num[%d]\n", frame->asdu->identify.vsq.continuous, frame->asdu->identify.vsq.num);
    printf("                  cot: %d\n", frame->asdu->identify.cot);
    printf("                  commonAddr: %d\n", frame->asdu->identify.commonAddr);
    switch (frame->asdu->identify.type)
    {
    case 0x01: {
        auto asdu1 = std::static_pointer_cast<npacket::iec103::Asdu1>(frame->asdu);
        if (asdu1)
        {
            printf("                  func: %d\n", asdu1->func);
            printf("                  inf: %d\n", asdu1->inf);
            printf("                  dpi: %d\n", asdu1->dpi);
            printf("                  time: %s\n", CP32Time2aToString(asdu1->tm).c_str());
            printf("                  sin: %d\n", asdu1->sin);
        }
    }
    break;
    case 0x02: {
        auto asdu2 = std::static_pointer_cast<npacket::iec103::Asdu2>(frame->asdu);
        if (asdu2)
        {
            printf("                  func: %d\n", asdu2->func);
            printf("                  inf: %d\n", asdu2->inf);
            printf("                  dpi: %d\n", asdu2->dpi);
            printf("                  ret: %d\n", asdu2->ret);
            printf("                  fan: %d\n", asdu2->fan);
            printf("                  time: %s\n", CP32Time2aToString(asdu2->tm).c_str());
            printf("                  sin: %d\n", asdu2->sin);
        }
    }
    break;
    case 0x03: {
        auto asdu3 = std::static_pointer_cast<npacket::iec103::Asdu3>(frame->asdu);
        if (asdu3)
        {
            printf("                  func: %d\n", asdu3->func);
            printf("                  inf: %d\n", asdu3->inf);
            for (size_t i = 0; i < asdu3->meaList.size(); ++i)
            {
                const auto& mea = asdu3->meaList[i];
                printf("                  mea[%03zu]: ov[%d], er[%d], res[%d], mval[%f]\n", i, mea.ov, mea.er, mea.res, mea.mval);
            }
        }
    }
    break;
    case 0x04: {
        auto asdu4 = std::static_pointer_cast<npacket::iec103::Asdu4>(frame->asdu);
        if (asdu4)
        {
            printf("                  func: %d\n", asdu4->func);
            printf("                  inf: %d\n", asdu4->inf);
            printf("                  scl: %f\n", asdu4->scl);
            printf("                  ret: %d\n", asdu4->ret);
            printf("                  fan: %d\n", asdu4->fan);
            printf("                  time: %s\n", CP32Time2aToString(asdu4->tm).c_str());
        }
    }
    break;
    case 0x05: {
        auto asdu5 = std::static_pointer_cast<npacket::iec103::Asdu5>(frame->asdu);
        if (asdu5)
        {
            printf("                  func: %d\n", asdu5->func);
            printf("                  inf: %d\n", asdu5->inf);
            printf("                  col: %d\n", asdu5->col);
            printf("                  ascii1: %d\n", asdu5->ascii1);
            printf("                  ascii2: %d\n", asdu5->ascii2);
            printf("                  ascii3: %d\n", asdu5->ascii3);
            printf("                  ascii4: %d\n", asdu5->ascii4);
            printf("                  ascii5: %d\n", asdu5->ascii5);
            printf("                  ascii6: %d\n", asdu5->ascii6);
            printf("                  ascii7: %d\n", asdu5->ascii7);
            printf("                  ascii8: %d\n", asdu5->ascii8);
            printf("                  freeValue1: %d\n", asdu5->freeValue1);
            printf("                  freeValue2: %d\n", asdu5->freeValue2);
            printf("                  freeValue3: %d\n", asdu5->freeValue3);
            printf("                  freeValue4: %d\n", asdu5->freeValue4);
        }
    }
    break;
    case 0x06: {
        auto asdu6 = std::static_pointer_cast<npacket::iec103::Asdu6>(frame->asdu);
        if (asdu6)
        {
            printf("                  func: %d\n", asdu6->func);
            printf("                  inf: %d\n", asdu6->inf);
            printf("                  time: %s\n", CP56Time2aToString(asdu6->tm).c_str());
        }
    }
    break;
    case 0x07: {
        auto asdu7 = std::static_pointer_cast<npacket::iec103::Asdu7>(frame->asdu);
        if (asdu7)
        {
            printf("                  func: %d\n", asdu7->func);
            printf("                  inf: %d\n", asdu7->inf);
            printf("                  scn: %d\n", asdu7->scn);
        }
    }
    break;
    case 0x08: {
        auto asdu8 = std::static_pointer_cast<npacket::iec103::Asdu8>(frame->asdu);
        if (asdu8)
        {
            printf("                  func: %d\n", asdu8->func);
            printf("                  inf: %d\n", asdu8->inf);
            printf("                  scn: %d\n", asdu8->scn);
        }
    }
    break;
    case 0x09: {
        auto asdu9 = std::static_pointer_cast<npacket::iec103::Asdu9>(frame->asdu);
        if (asdu9)
        {
            printf("                  func: %d\n", asdu9->func);
            printf("                  inf: %d\n", asdu9->inf);
            for (size_t i = 0; i < asdu9->meaList.size(); ++i)
            {
                const auto& mea = asdu9->meaList[i];
                printf("                  mea[%03zu]: ov[%d], er[%d], res[%d], mval[%f]\n", i, mea.ov, mea.er, mea.res, mea.mval);
            }
        }
    }
    break;
    case 0x0A: {
        auto asdu10 = std::static_pointer_cast<npacket::iec103::Asdu10>(frame->asdu);
        if (asdu10)
        {
            printf("                  func: %d\n", asdu10->func);
            printf("                  inf: %d\n", asdu10->inf);
            printf("                  rii: %d\n", asdu10->rii);
            printf("                  ngd: no[%d], count[%d], cont[%d]\n", asdu10->ngd.no, asdu10->ngd.count, asdu10->ngd.cont);
            for (size_t i = 0; i < asdu10->dataSet.size(); ++i)
            {
                const auto& dataSet10 = asdu10->dataSet[i];
                printf("                  dataSet[%03zu]: gin(group[%d], entry[%d])\n", i, dataSet10.gin.group, dataSet10.gin.entry);
                printf("                                kod[%d]\n", dataSet10.kod);
                printf("                                gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])\n", dataSet10.gdd.dataType,
                       dataSet10.gdd.dataSize, dataSet10.gdd.number, dataSet10.gdd.cont);
                for (size_t j = 0; j < dataSet10.gidList.size(); ++j)
                {
                    const auto& gid = dataSet10.gidList[j];
                    printf("                                gid[%03zu]: ", j);
                    switch (dataSet10.gdd.dataType)
                    {
                    case 1:
                        printf("ascii[%c]", gid.ascii);
                        break;
                    case 2:
                        printf("bsc[%d, %d, %d, %d]", gid.bsi[0], gid.bsi[1], gid.bsi[2], gid.bsi[3]);
                        break;
                    case 3:
                        printf("uintValue[%u]", gid.uintValue);
                        break;
                    case 4:
                        printf("intValue[%d]", gid.intValue);
                        break;
                    case 5:
                        printf("urealValue[%f]", gid.urealValue);
                        break;
                    case 6:
                        printf("realValue[%f]", gid.realValue);
                        break;
                    case 7:
                        printf("real32Value[%f]", gid.real32Value);
                        break;
                    case 8:
                        printf("real32Value[%f]", gid.real64Value);
                        break;
                    case 9:
                        printf("dpi[%d]", gid.dpi);
                        break;
                    case 10:
                        printf("spi[%d]", gid.spi);
                        break;
                    case 11:
                        printf("tedpi[%d]", gid.tedpi);
                        break;
                    case 12:
                        printf("mea(ov[%d], er[%d], res[%d], mval[%f])", gid.mea.ov, gid.mea.er, gid.mea.res, gid.mea.mval);
                        break;
                    case 14:
                        printf("time(%s)", CP56Time2aToString(gid.tm).c_str());
                        break;
                    case 15:
                        printf("gin(group[%d], entry[%d])", gid.gin.group, gid.gin.entry);
                        break;
                    case 16:
                        printf("ret[%d]", gid.ret);
                        break;
                    case 17:
                        printf("func[%d], inf[%d]", gid.func, gid.inf);
                        break;
                    case 18:
                        printf("dpi[%d], time(%s), sin[%d]", gid.dpiWithTime.dpi, CP32Time2aToString(gid.dpiWithTime.tm).c_str(),
                               gid.dpiWithTime.sin);
                        break;
                    case 19:
                        printf("dpi[%d], ret[%d], fan[%d], time(%s), sin[%d]", gid.dpiWithRet.dpi, gid.dpiWithRet.ret, gid.dpiWithRet.fan,
                               CP32Time2aToString(gid.dpiWithRet.tm).c_str(), gid.dpiWithRet.sin);
                        break;
                    case 20:
                        printf("real32Value[%f], ret[%d], fan[%d], time(%s)", gid.valWithRet.real32Value, gid.valWithRet.ret,
                               gid.valWithRet.fan, CP32Time2aToString(gid.valWithRet.tm).c_str());
                        break;
                    case 22:
                        printf("grc[%d]", gid.grc);
                        break;
                    case 23:
                        printf("gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])", gid.gdd.dataType, gid.gdd.dataSize, gid.gdd.number,
                               gid.gdd.cont);
                        break;
                    }
                    printf("\n");
                }
            }
        }
    }
    break;
    case 0x0B: {
        auto asdu11 = std::static_pointer_cast<npacket::iec103::Asdu11>(frame->asdu);
        if (asdu11)
        {
            printf("                  func: %d\n", asdu11->func);
            printf("                  inf: %d\n", asdu11->inf);
            printf("                  rii: %d\n", asdu11->rii);
            printf("                  gin: group[%d], entry[%d]\n", asdu11->gin.group, asdu11->gin.entry);
            printf("                  nde: no[%d], count[%d], cont[%d]\n", asdu11->nde.no, asdu11->nde.count, asdu11->nde.cont);
            for (size_t i = 0; i < asdu11->dataSet.size(); ++i)
            {
                const auto& dataSet11 = asdu11->dataSet[i];
                printf("                  dataSet[%03zu]: kod[%d]\n", i, dataSet11.kod);
                printf("                                gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])\n", dataSet11.gdd.dataType,
                       dataSet11.gdd.dataSize, dataSet11.gdd.number, dataSet11.gdd.cont);
                for (size_t j = 0; j < dataSet11.gidList.size(); ++j)
                {
                    const auto& gid = dataSet11.gidList[j];
                    printf("                                gid[%03zu]: ", j);
                    switch (dataSet11.gdd.dataType)
                    {
                    case 1:
                        printf("ascii[%c]", gid.ascii);
                        break;
                    case 2:
                        printf("bsc[%d, %d, %d, %d]", gid.bsi[0], gid.bsi[1], gid.bsi[2], gid.bsi[3]);
                        break;
                    case 3:
                        printf("uintValue[%u]", gid.uintValue);
                        break;
                    case 4:
                        printf("intValue[%d]", gid.intValue);
                        break;
                    case 5:
                        printf("urealValue[%f]", gid.urealValue);
                        break;
                    case 6:
                        printf("realValue[%f]", gid.realValue);
                        break;
                    case 7:
                        printf("real32Value[%f]", gid.real32Value);
                        break;
                    case 8:
                        printf("real32Value[%f]", gid.real64Value);
                        break;
                    case 9:
                        printf("dpi[%d]", gid.dpi);
                        break;
                    case 10:
                        printf("spi[%d]", gid.spi);
                        break;
                    case 11:
                        printf("tedpi[%d]", gid.tedpi);
                        break;
                    case 12:
                        printf("mea(ov[%d], er[%d], res[%d], mval[%f])", gid.mea.ov, gid.mea.er, gid.mea.res, gid.mea.mval);
                        break;
                    case 14:
                        printf("time(%s)", CP56Time2aToString(gid.tm).c_str());
                        break;
                    case 15:
                        printf("gin(group[%d], entry[%d])", gid.gin.group, gid.gin.entry);
                        break;
                    case 16:
                        printf("ret[%d]", gid.ret);
                        break;
                    case 17:
                        printf("func[%d], inf[%d]", gid.func, gid.inf);
                        break;
                    case 18:
                        printf("dpi[%d], time(%s), sin[%d]", gid.dpiWithTime.dpi, CP32Time2aToString(gid.dpiWithTime.tm).c_str(),
                               gid.dpiWithTime.sin);
                        break;
                    case 19:
                        printf("dpi[%d], ret[%d], fan[%d], time(%s), sin[%d]", gid.dpiWithRet.dpi, gid.dpiWithRet.ret, gid.dpiWithRet.fan,
                               CP32Time2aToString(gid.dpiWithRet.tm).c_str(), gid.dpiWithRet.sin);
                        break;
                    case 20:
                        printf("real32Value[%f], ret[%d], fan[%d], time(%s)", gid.valWithRet.real32Value, gid.valWithRet.ret,
                               gid.valWithRet.fan, CP32Time2aToString(gid.valWithRet.tm).c_str());
                        break;
                    case 22:
                        printf("grc[%d]", gid.grc);
                        break;
                    case 23:
                        printf("gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])", gid.gdd.dataType, gid.gdd.dataSize, gid.gdd.number,
                               gid.gdd.cont);
                        break;
                    }
                    printf("\n");
                }
            }
        }
    }
    break;
    case 0x14: {
        auto asdu20 = std::static_pointer_cast<npacket::iec103::Asdu20>(frame->asdu);
        if (asdu20)
        {
            printf("                  func: %d\n", asdu20->func);
            printf("                  inf: %d\n", asdu20->inf);
            printf("                  dco: %d\n", asdu20->dco);
            printf("                  rii: %d\n", asdu20->rii);
        }
    }
    break;
    case 0x15: {
        auto asdu21 = std::static_pointer_cast<npacket::iec103::Asdu21>(frame->asdu);
        if (asdu21)
        {
            printf("                  func: %d\n", asdu21->func);
            printf("                  inf: %d\n", asdu21->inf);
            printf("                  rii: %d\n", asdu21->rii);
            printf("                  nog: %d\n", asdu21->nog);
            for (size_t i = 0; i < asdu21->dataSet.size(); ++i)
            {
                const auto& dataSet21 = asdu21->dataSet[i];
                printf("                  dataSet[%03zu]: gin(group[%d], entry[%d]), kod(%d)\n", i, dataSet21.gin.group,
                       dataSet21.gin.entry, dataSet21.kod);
            }
        }
    }
    break;
    case 0x17: {
        auto asdu23 = std::static_pointer_cast<npacket::iec103::Asdu23>(frame->asdu);
        if (asdu23)
        {
            printf("                  func: %d\n", asdu23->func);
            for (size_t i = 0; i < asdu23->dataSet.size(); ++i)
            {
                const auto& dataSet23 = asdu23->dataSet[i];
                printf("                  dataSet[%03zu]: fan[%d]\n", i, dataSet23.fan);
                printf("                                sof(tp[%d], tm[%d], test[%d], otev[%d], res[%d])\n", dataSet23.sof.tp,
                       dataSet23.sof.tm, dataSet23.sof.test, dataSet23.sof.otev, dataSet23.sof.res);
                printf("                                time(%s)\n", CP56Time2aToString(dataSet23.tm).c_str());
            }
        }
    }
    break;
    case 0x18: {
        auto asdu24 = std::static_pointer_cast<npacket::iec103::Asdu24>(frame->asdu);
        if (asdu24)
        {
            printf("                  func: %d\n", asdu24->func);
            printf("                  too: %d\n", asdu24->too);
            printf("                  tov: %d\n", asdu24->tov);
            printf("                  fan: %d\n", asdu24->fan);
            printf("                  acc: %d\n", asdu24->acc);
        }
    }
    break;
    case 0x19: {
        auto asdu25 = std::static_pointer_cast<npacket::iec103::Asdu25>(frame->asdu);
        if (asdu25)
        {
            printf("                  func: %d\n", asdu25->func);
            printf("                  too: %d\n", asdu25->too);
            printf("                  tov: %d\n", asdu25->tov);
            printf("                  fan: %d\n", asdu25->fan);
            printf("                  acc: %d\n", asdu25->acc);
        }
    }
    break;
    case 0x1A: {
        auto asdu26 = std::static_pointer_cast<npacket::iec103::Asdu26>(frame->asdu);
        if (asdu26)
        {
            printf("                  func: %d\n", asdu26->func);
            printf("                  tov: %d\n", asdu26->tov);
            printf("                  fan: %d\n", asdu26->fan);
            printf("                  nof: %d\n", asdu26->nof);
            printf("                  noc: %d\n", asdu26->noc);
            printf("                  noe: %d\n", asdu26->noe);
            printf("                  interval: %d\n", asdu26->interval);
            printf("                  time: %s\n", CP32Time2aToString(asdu26->tm).c_str());
        }
    }
    break;
    case 0x1B: {
        auto asdu27 = std::static_pointer_cast<npacket::iec103::Asdu27>(frame->asdu);
        if (asdu27)
        {
            printf("                  func: %d\n", asdu27->func);
            printf("                  tov: %d\n", asdu27->tov);
            printf("                  fan: %d\n", asdu27->fan);
            printf("                  acc: %d\n", asdu27->acc);
            printf("                  rpv: %f\n", asdu27->rpv);
            printf("                  rsv: %f\n", asdu27->rsv);
            printf("                  rfa: %f\n", asdu27->rfa);
        }
    }
    break;
    case 0x1C: {
        auto asdu28 = std::static_pointer_cast<npacket::iec103::Asdu28>(frame->asdu);
        if (asdu28)
        {
            printf("                  func: %d\n", asdu28->func);
            printf("                  fan: %d\n", asdu28->fan);
        }
    }
    break;
    case 0x1D: {
        auto asdu29 = std::static_pointer_cast<npacket::iec103::Asdu29>(frame->asdu);
        if (asdu29)
        {
            printf("                  func: %d\n", asdu29->func);
            printf("                  fan: %d\n", asdu29->fan);
            printf("                  not: %d\n", asdu29->not );
            printf("                  tap: %d\n", asdu29->tap);
            for (size_t i = 0; i < asdu29->dataSet.size(); ++i)
            {
                const auto& dataSet29 = asdu29->dataSet[i];
                printf("                  dataSet[%03zu]: func[%d], inf[%d], dpi[%d]\n", i, dataSet29.func, dataSet29.inf, dataSet29.dpi);
            }
        }
    }
    break;
    case 0x1E: {
        auto asdu30 = std::static_pointer_cast<npacket::iec103::Asdu30>(frame->asdu);
        if (asdu30)
        {
            printf("                  func: %d\n", asdu30->func);
            printf("                  tov: %d\n", asdu30->tov);
            printf("                  fan: %d\n", asdu30->fan);
            printf("                  acc: %d\n", asdu30->acc);
            printf("                  ndv: %d\n", asdu30->ndv);
            printf("                  nfe: %d\n", asdu30->nfe);
            printf("                  sdv: ");
            for (size_t i = 0; i < asdu30->sdvList.size(); ++i)
            {
                if (0 != i)
                {
                    printf(", ");
                }
                printf("%f", asdu30->sdvList[i]);
            }
            printf("\n");
        }
    }
    break;
    case 0x1F: {
        auto asdu31 = std::static_pointer_cast<npacket::iec103::Asdu31>(frame->asdu);
        if (asdu31)
        {
            printf("                  func: %d\n", asdu31->func);
            printf("                  too: %d\n", asdu31->too);
            printf("                  tov: %d\n", asdu31->tov);
            printf("                  fan: %d\n", asdu31->fan);
            printf("                  acc: %d\n", asdu31->acc);
        }
    }
    break;
    case 0x26: {
        auto asdu38 = std::static_pointer_cast<npacket::iec103::Asdu38>(frame->asdu);
        if (asdu38)
        {
            printf("                  func: %d\n", asdu38->func);
            printf("                  inf: %d\n", asdu38->inf);
            printf("                  vti: type[%d], value[%d]\n", asdu38->vti.type, asdu38->vti.value);
            printf("                  qds: iv[%d], nt[%d], sb[%d], bl[%d], res[%d], ov[%d]\n", asdu38->qds.iv, asdu38->qds.nt,
                   asdu38->qds.sb, asdu38->qds.bl, asdu38->qds.res, asdu38->qds.ov);
        }
    }
    break;
    case 0x2A: {
        auto asdu42 = std::static_pointer_cast<npacket::iec103::Asdu42>(frame->asdu);
        if (asdu42)
        {
            printf("                  func: %d\n", asdu42->func);
            printf("                  inf: %d\n", asdu42->inf);
            printf("                  dpi: ");
            for (size_t i = 0; i < asdu42->dpiList.size(); ++i)
            {
                if (0 != i)
                {
                    printf(", ");
                }
                printf("%d", asdu42->dpiList[i]);
            }
            printf("\n");
            printf("                  sin: %d\n", asdu42->sin);
        }
    }
    break;
    }
}

void handleApplicationIec103FixedFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                       const std::shared_ptr<npacket::ProtocolHeader>& header,
                                       const std::shared_ptr<npacket::iec103::FixedFrame>& frame)
{
    if (Filter::getInstance().showIec103())
    {
        printTransportHeader(header);
        printIEC103FixedFrame(frame);
    }
}

void handleApplicationIec103VariableFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                          const std::shared_ptr<npacket::ProtocolHeader>& header,
                                          const std::shared_ptr<npacket::iec103::VariableFrame>& frame)
{
    if (Filter::getInstance().showIec103())
    {
        printTransportHeader(header);
        printIEC103VariableFrame(frame);
    }
}
