#!/bin/bash 
# ARCHS="BASIC_FX32_DMA32_TYPE0 BASIC_FX32_DMA64_TYPE0 BASIC_FX32_DMA32_TYPE1 BASIC_FX32_DMA64_TYPE1"
ARCHS=" BASIC_FX28_DMA28_TYPE1 BASIC_FX24_DMA48_TYPE1 BASIC_FX24_DMA32_TYPE1 BASIC_FX24_DMA64_TYPE1 BASIC_FX32_DMA32_TYPE0 BASIC_FX32_DMA64_TYPE0 BASIC_FX32_DMA32_TYPE1 BASIC_FX32_DMA64_TYPE1"

for arc in $ARCHS; do

    # Get latency of each architecture
    LATENCY_TRACE="${PWD}/bdw_work/trace/sim.${arc}_V.trace"
    LATENCY_LOG="${PWD}/latency_${arc}.log"
    
    if test ! -e $LATENCY_TRACE; then
        echo "--- $LATENCY_TRACE not found ---"
        continue
    fi

    LIST_INFO_DRIVER=$(cat $LATENCY_TRACE | grep "Info: testbench: @")
    ARR_INFO_DRIVER=($LIST_INFO_DRIVER)
    
    # == NOTE: the numbers for time beg/end have to be modified if the format is changed
    # use //@ to remove the @ character at the front
    T_END=${ARR_INFO_DRIVER[50]//@}
    echo "end time: ${T_END}"
    T_BEG=${ARR_INFO_DRIVER[2]//@}
    echo "begin time: ${T_BEG}"

	#     TIME_BEG=${ARR_INFO_DRIVER[8]//@}
	#     TIME_END_SCIENCE=${ARR_INFO_DRIVER[15]//@}
	#     TIME_END=$(printf "%0f" "${TIME_END_SCIENCE}")
    TOT_TIME=$(echo "${T_END} - ${T_BEG}" | bc)
    
    echo "${TOT_TIME}" > $LATENCY_LOG
done
