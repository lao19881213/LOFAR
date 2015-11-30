# Quick way of priming the pipeline with a list of datafiles to be processed.
# Generate this file by, eg:
#
# $ obsid=L2010_08322; for i in `seq 0 7`; do ssh lce`printf %03d $((i*9+1))` ls /net/sub$((i+1))/lse*/data*/$obsid/* | grep SB >> to_process.py; done
#
# then tweak with your favourite text editor.

datafiles = [
'/net/sub1/lse001/data3/L2010_08567/SB0.MS',
'/net/sub1/lse001/data3/L2010_08567/SB10.MS',
'/net/sub1/lse001/data3/L2010_08567/SB11.MS',
'/net/sub1/lse001/data3/L2010_08567/SB12.MS',
'/net/sub1/lse001/data3/L2010_08567/SB13.MS',
'/net/sub1/lse001/data3/L2010_08567/SB14.MS',
'/net/sub1/lse001/data3/L2010_08567/SB15.MS',
'/net/sub1/lse001/data3/L2010_08567/SB16.MS',
'/net/sub1/lse001/data3/L2010_08567/SB1.MS',
'/net/sub1/lse001/data3/L2010_08567/SB2.MS',
'/net/sub1/lse001/data3/L2010_08567/SB3.MS',
'/net/sub1/lse001/data3/L2010_08567/SB4.MS',
'/net/sub1/lse001/data3/L2010_08567/SB5.MS',
'/net/sub1/lse001/data3/L2010_08567/SB6.MS',
'/net/sub1/lse001/data3/L2010_08567/SB7.MS',
'/net/sub1/lse001/data3/L2010_08567/SB8.MS',
'/net/sub1/lse001/data3/L2010_08567/SB9.MS',
'/net/sub1/lse002/data3/L2010_08567/SB17.MS',
'/net/sub1/lse002/data3/L2010_08567/SB18.MS',
'/net/sub1/lse002/data3/L2010_08567/SB19.MS',
'/net/sub1/lse002/data3/L2010_08567/SB20.MS',
'/net/sub1/lse002/data3/L2010_08567/SB21.MS',
'/net/sub1/lse002/data3/L2010_08567/SB22.MS',
'/net/sub1/lse002/data3/L2010_08567/SB23.MS',
'/net/sub1/lse002/data3/L2010_08567/SB24.MS',
'/net/sub1/lse002/data3/L2010_08567/SB25.MS',
'/net/sub1/lse002/data3/L2010_08567/SB26.MS',
'/net/sub1/lse002/data3/L2010_08567/SB27.MS',
'/net/sub1/lse002/data3/L2010_08567/SB28.MS',
'/net/sub1/lse002/data3/L2010_08567/SB29.MS',
'/net/sub1/lse002/data3/L2010_08567/SB30.MS',
'/net/sub1/lse002/data3/L2010_08567/SB31.MS',
'/net/sub1/lse002/data3/L2010_08567/SB32.MS',
'/net/sub1/lse002/data3/L2010_08567/SB33.MS',
'/net/sub1/lse003/data3/L2010_08567/SB34.MS',
'/net/sub1/lse003/data3/L2010_08567/SB35.MS',
'/net/sub1/lse003/data3/L2010_08567/SB36.MS',
'/net/sub1/lse003/data3/L2010_08567/SB37.MS',
'/net/sub1/lse003/data3/L2010_08567/SB38.MS',
'/net/sub1/lse003/data3/L2010_08567/SB39.MS',
'/net/sub1/lse003/data3/L2010_08567/SB40.MS',
'/net/sub1/lse003/data3/L2010_08567/SB41.MS',
'/net/sub1/lse003/data3/L2010_08567/SB42.MS',
'/net/sub1/lse003/data3/L2010_08567/SB43.MS',
'/net/sub1/lse003/data3/L2010_08567/SB44.MS',
'/net/sub1/lse003/data3/L2010_08567/SB45.MS',
'/net/sub1/lse003/data3/L2010_08567/SB46.MS',
'/net/sub1/lse003/data3/L2010_08567/SB47.MS',
'/net/sub1/lse003/data3/L2010_08567/SB48.MS',
'/net/sub1/lse003/data3/L2010_08567/SB49.MS',
'/net/sub1/lse003/data3/L2010_08567/SB50.MS',
'/net/sub3/lse007/data3/L2010_08567/SB51.MS',
'/net/sub3/lse007/data3/L2010_08567/SB52.MS',
'/net/sub3/lse007/data3/L2010_08567/SB53.MS',
'/net/sub3/lse007/data3/L2010_08567/SB54.MS',
'/net/sub3/lse007/data3/L2010_08567/SB55.MS',
'/net/sub3/lse007/data3/L2010_08567/SB56.MS',
'/net/sub3/lse007/data3/L2010_08567/SB57.MS',
'/net/sub3/lse007/data3/L2010_08567/SB58.MS',
'/net/sub3/lse007/data3/L2010_08567/SB59.MS',
'/net/sub3/lse007/data3/L2010_08567/SB60.MS',
'/net/sub3/lse007/data3/L2010_08567/SB61.MS',
'/net/sub3/lse007/data3/L2010_08567/SB62.MS',
'/net/sub3/lse007/data3/L2010_08567/SB63.MS',
'/net/sub3/lse007/data3/L2010_08567/SB64.MS',
'/net/sub3/lse007/data3/L2010_08567/SB65.MS',
'/net/sub3/lse007/data3/L2010_08567/SB66.MS',
'/net/sub3/lse007/data3/L2010_08567/SB67.MS',
'/net/sub3/lse008/data3/L2010_08567/SB68.MS',
'/net/sub3/lse008/data3/L2010_08567/SB69.MS',
'/net/sub3/lse008/data3/L2010_08567/SB70.MS',
'/net/sub3/lse008/data3/L2010_08567/SB71.MS',
'/net/sub3/lse008/data3/L2010_08567/SB72.MS',
'/net/sub3/lse008/data3/L2010_08567/SB73.MS',
'/net/sub3/lse008/data3/L2010_08567/SB74.MS',
'/net/sub3/lse008/data3/L2010_08567/SB75.MS',
'/net/sub3/lse008/data3/L2010_08567/SB76.MS',
'/net/sub3/lse008/data3/L2010_08567/SB77.MS',
'/net/sub3/lse008/data3/L2010_08567/SB78.MS',
'/net/sub3/lse008/data3/L2010_08567/SB79.MS',
'/net/sub3/lse008/data3/L2010_08567/SB80.MS',
'/net/sub3/lse008/data3/L2010_08567/SB81.MS',
'/net/sub3/lse008/data3/L2010_08567/SB82.MS',
'/net/sub3/lse008/data3/L2010_08567/SB83.MS',
'/net/sub3/lse008/data3/L2010_08567/SB84.MS',
'/net/sub3/lse009/data3/L2010_08567/SB100.MS',
'/net/sub3/lse009/data3/L2010_08567/SB101.MS',
'/net/sub3/lse009/data3/L2010_08567/SB85.MS',
'/net/sub3/lse009/data3/L2010_08567/SB86.MS',
'/net/sub3/lse009/data3/L2010_08567/SB87.MS',
'/net/sub3/lse009/data3/L2010_08567/SB88.MS',
'/net/sub3/lse009/data3/L2010_08567/SB89.MS',
'/net/sub3/lse009/data3/L2010_08567/SB90.MS',
'/net/sub3/lse009/data3/L2010_08567/SB91.MS',
'/net/sub3/lse009/data3/L2010_08567/SB92.MS',
'/net/sub3/lse009/data3/L2010_08567/SB93.MS',
'/net/sub3/lse009/data3/L2010_08567/SB94.MS',
'/net/sub3/lse009/data3/L2010_08567/SB95.MS',
'/net/sub3/lse009/data3/L2010_08567/SB96.MS',
'/net/sub3/lse009/data3/L2010_08567/SB97.MS',
'/net/sub3/lse009/data3/L2010_08567/SB98.MS',
'/net/sub3/lse009/data3/L2010_08567/SB99.MS',
'/net/sub4/lse010/data3/L2010_08567/SB102.MS',
'/net/sub4/lse010/data3/L2010_08567/SB103.MS',
'/net/sub4/lse010/data3/L2010_08567/SB104.MS',
'/net/sub4/lse010/data3/L2010_08567/SB105.MS',
'/net/sub4/lse010/data3/L2010_08567/SB106.MS',
'/net/sub4/lse010/data3/L2010_08567/SB107.MS',
'/net/sub4/lse010/data3/L2010_08567/SB108.MS',
'/net/sub4/lse010/data3/L2010_08567/SB109.MS',
'/net/sub4/lse010/data3/L2010_08567/SB110.MS',
'/net/sub4/lse010/data3/L2010_08567/SB111.MS',
'/net/sub4/lse010/data3/L2010_08567/SB112.MS',
'/net/sub4/lse010/data3/L2010_08567/SB113.MS',
'/net/sub4/lse010/data3/L2010_08567/SB114.MS',
'/net/sub4/lse010/data3/L2010_08567/SB115.MS',
'/net/sub4/lse010/data3/L2010_08567/SB116.MS',
'/net/sub4/lse010/data3/L2010_08567/SB117.MS',
'/net/sub4/lse010/data3/L2010_08567/SB118.MS',
'/net/sub4/lse011/data3/L2010_08567/SB119.MS',
'/net/sub4/lse011/data3/L2010_08567/SB120.MS',
'/net/sub4/lse011/data3/L2010_08567/SB121.MS',
'/net/sub4/lse011/data3/L2010_08567/SB122.MS',
'/net/sub4/lse011/data3/L2010_08567/SB123.MS',
'/net/sub4/lse011/data3/L2010_08567/SB124.MS',
'/net/sub4/lse011/data3/L2010_08567/SB125.MS',
'/net/sub4/lse011/data3/L2010_08567/SB126.MS',
'/net/sub4/lse011/data3/L2010_08567/SB127.MS',
'/net/sub4/lse011/data3/L2010_08567/SB128.MS',
'/net/sub4/lse011/data3/L2010_08567/SB129.MS',
'/net/sub4/lse011/data3/L2010_08567/SB130.MS',
'/net/sub4/lse011/data3/L2010_08567/SB131.MS',
'/net/sub4/lse011/data3/L2010_08567/SB132.MS',
'/net/sub4/lse011/data3/L2010_08567/SB133.MS',
'/net/sub4/lse011/data3/L2010_08567/SB134.MS',
'/net/sub4/lse011/data3/L2010_08567/SB135.MS',
'/net/sub4/lse012/data3/L2010_08567/SB136.MS',
'/net/sub4/lse012/data3/L2010_08567/SB137.MS',
'/net/sub4/lse012/data3/L2010_08567/SB138.MS',
'/net/sub4/lse012/data3/L2010_08567/SB139.MS',
'/net/sub4/lse012/data3/L2010_08567/SB140.MS',
'/net/sub4/lse012/data3/L2010_08567/SB141.MS',
'/net/sub4/lse012/data3/L2010_08567/SB142.MS',
'/net/sub4/lse012/data3/L2010_08567/SB143.MS',
'/net/sub4/lse012/data3/L2010_08567/SB144.MS',
'/net/sub4/lse012/data3/L2010_08567/SB145.MS',
'/net/sub4/lse012/data3/L2010_08567/SB146.MS',
'/net/sub4/lse012/data3/L2010_08567/SB147.MS',
'/net/sub4/lse012/data3/L2010_08567/SB148.MS',
'/net/sub4/lse012/data3/L2010_08567/SB149.MS',
'/net/sub4/lse012/data3/L2010_08567/SB150.MS',
'/net/sub4/lse012/data3/L2010_08567/SB151.MS',
'/net/sub4/lse012/data3/L2010_08567/SB152.MS',
'/net/sub6/lse016/data3/L2010_08567/SB153.MS',
'/net/sub6/lse016/data3/L2010_08567/SB154.MS',
'/net/sub6/lse016/data3/L2010_08567/SB155.MS',
'/net/sub6/lse016/data3/L2010_08567/SB156.MS',
'/net/sub6/lse016/data3/L2010_08567/SB157.MS',
'/net/sub6/lse016/data3/L2010_08567/SB158.MS',
'/net/sub6/lse016/data3/L2010_08567/SB159.MS',
'/net/sub6/lse016/data3/L2010_08567/SB160.MS',
'/net/sub6/lse016/data3/L2010_08567/SB161.MS',
'/net/sub6/lse016/data3/L2010_08567/SB162.MS',
'/net/sub6/lse016/data3/L2010_08567/SB163.MS',
'/net/sub6/lse016/data3/L2010_08567/SB164.MS',
'/net/sub6/lse016/data3/L2010_08567/SB165.MS',
'/net/sub6/lse016/data3/L2010_08567/SB166.MS',
'/net/sub6/lse016/data3/L2010_08567/SB167.MS',
'/net/sub6/lse016/data3/L2010_08567/SB168.MS',
'/net/sub6/lse016/data3/L2010_08567/SB169.MS',
'/net/sub6/lse017/data3/L2010_08567/SB170.MS',
'/net/sub6/lse017/data3/L2010_08567/SB171.MS',
'/net/sub6/lse017/data3/L2010_08567/SB172.MS',
'/net/sub6/lse017/data3/L2010_08567/SB173.MS',
'/net/sub6/lse017/data3/L2010_08567/SB174.MS',
'/net/sub6/lse017/data3/L2010_08567/SB175.MS',
'/net/sub6/lse017/data3/L2010_08567/SB176.MS',
'/net/sub6/lse017/data3/L2010_08567/SB177.MS',
'/net/sub6/lse017/data3/L2010_08567/SB178.MS',
'/net/sub6/lse017/data3/L2010_08567/SB179.MS',
'/net/sub6/lse017/data3/L2010_08567/SB180.MS',
'/net/sub6/lse017/data3/L2010_08567/SB181.MS',
'/net/sub6/lse017/data3/L2010_08567/SB182.MS',
'/net/sub6/lse017/data3/L2010_08567/SB183.MS',
'/net/sub6/lse017/data3/L2010_08567/SB184.MS',
'/net/sub6/lse017/data3/L2010_08567/SB185.MS',
'/net/sub6/lse017/data3/L2010_08567/SB186.MS',
'/net/sub6/lse018/data3/L2010_08567/SB187.MS',
'/net/sub6/lse018/data3/L2010_08567/SB188.MS',
'/net/sub6/lse018/data3/L2010_08567/SB189.MS',
'/net/sub6/lse018/data3/L2010_08567/SB190.MS',
'/net/sub6/lse018/data3/L2010_08567/SB191.MS',
'/net/sub6/lse018/data3/L2010_08567/SB192.MS',
'/net/sub6/lse018/data3/L2010_08567/SB193.MS',
'/net/sub6/lse018/data3/L2010_08567/SB194.MS',
'/net/sub6/lse018/data3/L2010_08567/SB195.MS',
'/net/sub6/lse018/data3/L2010_08567/SB196.MS',
'/net/sub6/lse018/data3/L2010_08567/SB197.MS',
'/net/sub6/lse018/data3/L2010_08567/SB198.MS',
'/net/sub6/lse018/data3/L2010_08567/SB199.MS',
'/net/sub6/lse018/data3/L2010_08567/SB200.MS',
'/net/sub6/lse018/data3/L2010_08567/SB201.MS',
'/net/sub6/lse018/data3/L2010_08567/SB202.MS',
'/net/sub6/lse018/data3/L2010_08567/SB203.MS',
'/net/sub8/lse022/data3/L2010_08567/SB204.MS',
'/net/sub8/lse022/data3/L2010_08567/SB205.MS',
'/net/sub8/lse022/data3/L2010_08567/SB206.MS',
'/net/sub8/lse022/data3/L2010_08567/SB207.MS',
'/net/sub8/lse022/data3/L2010_08567/SB208.MS',
'/net/sub8/lse022/data3/L2010_08567/SB209.MS',
'/net/sub8/lse022/data3/L2010_08567/SB210.MS',
'/net/sub8/lse022/data3/L2010_08567/SB211.MS',
'/net/sub8/lse022/data3/L2010_08567/SB212.MS',
'/net/sub8/lse022/data3/L2010_08567/SB213.MS',
'/net/sub8/lse022/data3/L2010_08567/SB214.MS',
'/net/sub8/lse022/data3/L2010_08567/SB215.MS',
'/net/sub8/lse022/data3/L2010_08567/SB216.MS',
'/net/sub8/lse022/data3/L2010_08567/SB217.MS',
'/net/sub8/lse022/data3/L2010_08567/SB218.MS',
'/net/sub8/lse022/data3/L2010_08567/SB219.MS',
'/net/sub8/lse022/data3/L2010_08567/SB220.MS',
'/net/sub8/lse023/data3/L2010_08567/SB221.MS',
'/net/sub8/lse023/data3/L2010_08567/SB222.MS',
'/net/sub8/lse023/data3/L2010_08567/SB223.MS',
'/net/sub8/lse023/data3/L2010_08567/SB224.MS',
'/net/sub8/lse023/data3/L2010_08567/SB225.MS',
'/net/sub8/lse023/data3/L2010_08567/SB226.MS',
'/net/sub8/lse023/data3/L2010_08567/SB227.MS',
'/net/sub8/lse023/data3/L2010_08567/SB228.MS',
'/net/sub8/lse023/data3/L2010_08567/SB229.MS',
'/net/sub8/lse023/data3/L2010_08567/SB230.MS',
'/net/sub8/lse023/data3/L2010_08567/SB231.MS',
'/net/sub8/lse023/data3/L2010_08567/SB232.MS',
'/net/sub8/lse023/data3/L2010_08567/SB233.MS',
'/net/sub8/lse023/data3/L2010_08567/SB234.MS',
'/net/sub8/lse023/data3/L2010_08567/SB235.MS',
'/net/sub8/lse023/data3/L2010_08567/SB236.MS',
'/net/sub8/lse023/data3/L2010_08567/SB237.MS',
]
