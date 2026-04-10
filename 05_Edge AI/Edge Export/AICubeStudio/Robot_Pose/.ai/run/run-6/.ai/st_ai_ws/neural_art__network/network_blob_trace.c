#include "ll_aton_NN_interface.h"
#include "ll_aton.h"
#include "ll_aton_ec_trace.h"

#if 0
// Workaround: the tracer does not know the target at this moment
// and cannot call the functions since are used in static code
#define ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(address) LL_Address_Physical2Virtual(address)
#define ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(address) LL_Address_Virtual2Physical(address)
#else
#define ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(address) (address)
#define ATON_LIB_VIRTUAL_TO_PHYSICAL_ADDR(address) (address)
#endif


// MCU cache line size: 32 (bytes)
// NPU cache line size: 64 (bytes)
// MCU+NPU cache line size equal to 64 bytes (power of 2 not less than 8)
unsigned int cache_line_size = 64;

mpool_reloc_info_t mpool_reloc_info[] = {
  {"AXISRAM6", "_mem_pool_AXISRAM6_network", 0x34350000, 1, 0},
  {"AXISRAM5", "_mem_pool_AXISRAM5_network", 0x342e0000, 1, 0},
  {"AXISRAM4", "_mem_pool_AXISRAM4_network", 0x34270000, 1, 0},
  {"AXISRAM3", "_mem_pool_AXISRAM3_network", 0x34200000, 1, 0},
  {"AXISRAM2", "_mem_pool_AXISRAM2_network", 0x34100000, 1, 0},
  {"AXISRAM1", "_mem_pool_AXISRAM1_network", 0x34064000, 1, 0},
  {"AXIFLEXMEM", "_mem_pool_AXIFLEXMEM_network", 0x34000000, 1, 0},
  {"xSPI1", "_mem_pool_xSPI1_network", 0x90000000, 1, 0},
  {"xSPI2", "_mem_pool_xSPI2_network", 0x71000000, 1, 0},
  {"AXISRAM2_AXISRAM3_AXISRAM4_AXISRAM5_AXISRAM6", "_mem_pool_AXISRAM2_AXISRAM3_AXISRAM4_AXISRAM5_AXISRAM6_network", 0x34100000, 1, 0},
  {NULL, NULL, 0, 0, 0}
};


void trace_ec__ec_blob_network_1(void) {
  ec_trace_start_blob("_ec_blob_network_1");
  ec_trace_start_epoch(1);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id780 */
    /* node=Identity_inserted_id780 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id780 input ports=0 range=7[0,4915200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id780_dma_init_in_0_1 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Input_6_out_0_inserted_in780 */
      .offset_start = 0,
      .offset_end = 1638400,
      .offset_limit = 4915264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1638400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Identity_inserted_id780_dma_init_in_0_1, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 4915200 */

    /* Dma output units from cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id780 output ports=0 range=7[4915200,9830400] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id780_dma_init_out_0_1 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Input_6_out_0_inserted_out780 */
      .offset_start = 4915200,
      .offset_limit = 9830464,
      .frame_count = 0,
      .fwidth = 640,
      .fheight = 640,
      .batch_depth = 2,
      .batch_offset = 12,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 4915200,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Identity_inserted_id780_dma_init_out_0_1, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 4915200 */

    static const LL_Switch_InitTypeDef switch_init_in_1[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id780 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=1 */
    LL_Switch_Init(switch_init_in_1, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_1_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_1_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x10);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_1[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id780 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=1 */
    LL_Switch_Deinit(switch_deinit_in_1, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_1_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_1_all_units, 2);

  }
  ec_trace_end_epoch(1);
  ec_trace_end_blob("_ec_blob_network_1");
}

void trace_ec__ec_blob_network_8(void) {
  ec_trace_start_blob("_ec_blob_network_8");
  ec_trace_start_epoch(8);
  {
  }
  {
  }
  ec_trace_end_epoch(8);
  ec_trace_end_blob("_ec_blob_network_8");
}

void trace_ec__ec_blob_network_11(void) {
  ec_trace_start_blob("_ec_blob_network_11");
  ec_trace_start_epoch(11);
  {
  }
  {
  }
  ec_trace_end_epoch(11);
  ec_trace_end_blob("_ec_blob_network_11");
}

void trace_ec__ec_blob_network_22(void) {
  ec_trace_start_blob("_ec_blob_network_22");
  ec_trace_start_epoch(22);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_24 */
    /* node=Concat_24 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_24 input ports=0 range=7[0,4915200] */

    static const LL_Streng_TensorInitTypeDef Concat_24_dma_init_in_0_22 = {
      /* from memory with batch=16 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_16_out_0 */
      .offset_start = 0,
      .offset_end = 1638400,
      .offset_limit = 4915264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1638400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Concat_24_dma_init_in_0_22, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 4915200 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_24 output ports=0 range=7[4915200,9830400] */

    static const LL_Streng_TensorInitTypeDef Concat_24_dma_init_out_0_22 = {
      /* to memory canonical from batch=16 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Concat_24_out_0 */
      .offset_start = 4915200,
      .offset_limit = 9830464,
      .frame_count = 0,
      .fwidth = 160,
      .fheight = 160,
      .batch_depth = 32,
      .batch_offset = 192,
      .frame_offset = 64,
      .line_offset = 0,
      .loop_offset = 4915200,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Concat_24_dma_init_out_0_22, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 4915200 */

    static const LL_Switch_InitTypeDef switch_init_in_22[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_24 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=22 */
    LL_Switch_Init(switch_init_in_22, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_22_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_22_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_22[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_24 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=22 */
    LL_Switch_Deinit(switch_deinit_in_22, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_22_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_22_all_units, 2);

  }
  ec_trace_end_epoch(22);
  ec_trace_end_blob("_ec_blob_network_22");
}

void trace_ec__ec_blob_network_40(void) {
  ec_trace_start_blob("_ec_blob_network_40");
  ec_trace_start_epoch(40);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_42 */
    /* node=Concat_42 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_42 input ports=0 range=7[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_42_dma_init_in_0_40 = {
      /* from memory with batch=32 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_34_out_0 */
      .offset_start = 0,
      .offset_end = 819200,
      .offset_limit = 2457664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 819200,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_42_dma_init_in_0_40, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2457600 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_42 output ports=0 range=11[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_42_dma_init_out_0_40 = {
      /* to memory canonical from batch=32 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_42_out_0 */
      .offset_start = 0,
      .offset_limit = 2457664,
      .frame_count = 0,
      .fwidth = 80,
      .fheight = 80,
      .batch_depth = 64,
      .batch_offset = 384,
      .frame_offset = 128,
      .line_offset = 0,
      .loop_offset = 2457600,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Concat_42_dma_init_out_0_40, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM6 <- 32768 */
    /* npuRAM5 <- 458752 */
    /* npuRAM4 <- 458752 */
    /* npuRAM3 <- 458752 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_40[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_42 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=40 */
    LL_Switch_Init(switch_init_in_40, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_40_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_40_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x40);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_40[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_42 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=40 */
    LL_Switch_Deinit(switch_deinit_in_40, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_40_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_40_all_units, 2);

  }
  ec_trace_end_epoch(40);
  ec_trace_end_blob("_ec_blob_network_40");
}

void trace_ec__ec_blob_network_71(void) {
  ec_trace_start_blob("_ec_blob_network_71");
  ec_trace_start_epoch(71);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_73 */
    /* node=Concat_73 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_73 input ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_73_dma_init_in_0_71 = {
      /* from memory with batch=32 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Add_72_out_0 */
      .offset_start = 0,
      .offset_end = 204800,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 204800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Concat_73_dma_init_in_0_71, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_73 output ports=0 range=2[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_73_dma_init_out_0_71 = {
      /* to memory canonical from batch=32 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Concat_73_out_0 */
      .offset_start = 0,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 64,
      .batch_offset = 256,
      .frame_offset = 128,
      .line_offset = 0,
      .loop_offset = 409600,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Concat_73_dma_init_out_0_71, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 409600 */

    static const LL_Switch_InitTypeDef switch_init_in_71[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_73 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=71 */
    LL_Switch_Init(switch_init_in_71, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_71_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_71_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_71[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_73 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=71 */
    LL_Switch_Deinit(switch_deinit_in_71, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_71_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_71_all_units, 2);

  }
  ec_trace_end_epoch(71);
  ec_trace_end_blob("_ec_blob_network_71");
}

void trace_ec__ec_blob_network_75(void) {
  ec_trace_start_blob("_ec_blob_network_75");
  ec_trace_start_epoch(75);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_77 */
    /* node=Concat_77 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_77 input ports=0 range=7[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_77_dma_init_in_0_75 = {
      /* from memory with batch=64 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_52_out_0 */
      .offset_start = 0,
      .offset_end = 409600,
      .offset_limit = 1228864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 409600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_77_dma_init_in_0_75, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1228800 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_77 output ports=0 range=11[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_77_dma_init_out_0_75 = {
      /* to memory canonical from batch=64 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_77_out_0 */
      .offset_start = 0,
      .offset_limit = 1228864,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 128,
      .batch_offset = 768,
      .frame_offset = 256,
      .line_offset = 0,
      .loop_offset = 1228800,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Concat_77_dma_init_out_0_75, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM3 <- 180224 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_75[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_77 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=75 */
    LL_Switch_Init(switch_init_in_75, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_75_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_75_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x40);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_75[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_77 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=75 */
    LL_Switch_Deinit(switch_deinit_in_75, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_75_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_75_all_units, 2);

  }
  ec_trace_end_epoch(75);
  ec_trace_end_blob("_ec_blob_network_75");
}

void trace_ec__ec_blob_network_106(void) {
  ec_trace_start_blob("_ec_blob_network_106");
  ec_trace_start_epoch(106);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_108 */
    /* node=Concat_108 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_108 input ports=0 range=1[0,204800] */

    static const LL_Streng_TensorInitTypeDef Concat_108_dma_init_in_0_106 = {
      /* from memory with batch=64 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Add_107_out_0 */
      .offset_start = 0,
      .offset_end = 102400,
      .offset_limit = 204864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 102400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Concat_108_dma_init_in_0_106, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_108 output ports=0 range=1[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_108_dma_init_out_0_106 = {
      /* to memory canonical from batch=64 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Concat_108_out_0 */
      .offset_start = 204800,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 128,
      .batch_offset = 512,
      .frame_offset = 256,
      .line_offset = 0,
      .loop_offset = 204800,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Concat_108_dma_init_out_0_106, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_106[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_108 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=106 */
    LL_Switch_Init(switch_init_in_106, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_106_all_units[] = {
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_106_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x20);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_106[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_108 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=106 */
    LL_Switch_Deinit(switch_deinit_in_106, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_106_all_units[] = {
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_106_all_units, 2);

  }
  ec_trace_end_epoch(106);
  ec_trace_end_blob("_ec_blob_network_106");
}

void trace_ec__ec_blob_network_110(void) {
  ec_trace_start_blob("_ec_blob_network_110");
  ec_trace_start_epoch(110);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_112 */
    /* node=Concat_112 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_112 input ports=0 range=11[614400,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_112_dma_init_in_0_110 = {
      /* from memory with batch=128 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Split_87_out_0 */
      .offset_start = 614400,
      .offset_end = 819200,
      .offset_limit = 1228864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 204800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(7, &Concat_112_dma_init_in_0_110, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM3 -> 180224 */
    /* cpuRAM2 -> 434176 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_112 output ports=0 range=11[0,614400] */

    static const LL_Streng_TensorInitTypeDef Concat_112_dma_init_out_0_110 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_112_out_0 */
      .offset_start = 0,
      .offset_limit = 614464,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 256,
      .batch_offset = 1536,
      .frame_offset = 512,
      .line_offset = 0,
      .loop_offset = 614400,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Concat_112_dma_init_out_0_110, 1);


    /* Dma output bandwidth to memory pools: */
    /* cpuRAM2 <- 614400 */

    static const LL_Switch_InitTypeDef switch_init_in_110[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_112 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=110 */
    LL_Switch_Init(switch_init_in_110, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_110_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_110_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x4);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_110[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_112 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=110 */
    LL_Switch_Deinit(switch_deinit_in_110, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_110_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_110_all_units, 2);

  }
  ec_trace_end_epoch(110);
  ec_trace_end_blob("_ec_blob_network_110");
}

void trace_ec__ec_blob_network_124(void) {
  ec_trace_start_blob("_ec_blob_network_124");
  ec_trace_start_epoch(124);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_120 */
    /* node=Concat_120 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_120 input ports=0 range=7[0,819200] */

    static const LL_Streng_TensorInitTypeDef Concat_120_dma_init_in_0_124 = {
      /* from memory with batch=128 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Conv2D_116_out_0 */
      .offset_start = 0,
      .offset_end = 204800,
      .offset_limit = 819264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 204800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_120_dma_init_in_0_124, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 819200 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_120 output ports=0 range=11[0,819200] */

    static const LL_Streng_TensorInitTypeDef Concat_120_dma_init_out_0_124 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_120_out_0 */
      .offset_start = 0,
      .offset_limit = 819264,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 256,
      .batch_offset = 2048,
      .frame_offset = 512,
      .line_offset = 0,
      .loop_offset = 819200,
      .frame_loop_cnt = 4,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Concat_120_dma_init_out_0_124, 1);


    /* Dma output bandwidth to memory pools: */
    /* cpuRAM2 <- 819200 */

    static const LL_Switch_InitTypeDef switch_init_in_124[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_120 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=124 */
    LL_Switch_Init(switch_init_in_124, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_124_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_124_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x40);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_124[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_120 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=124 */
    LL_Switch_Deinit(switch_deinit_in_124, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_124_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_124_all_units, 2);

  }
  ec_trace_end_epoch(124);
  ec_trace_end_blob("_ec_blob_network_124");
}

void trace_ec__ec_blob_network_133(void) {
  ec_trace_start_blob("_ec_blob_network_133");
  ec_trace_start_epoch(133);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_129 */
    /* node=Reshape_129 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_129 input ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_129_dma_init_in_0_133 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Conv2D_128_out_0 */
      .offset_start = 0,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 1024,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 409600,
      .frame_loop_cnt = 256,
      .frame_tot_cnt = 256,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Reshape_129_dma_init_in_0_133, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_129 output ports=0 range=2[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_129_dma_init_out_0_133 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_129_out_0 */
      .offset_start = 0,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 128,
      .batch_depth = 2,
      .batch_offset = 8,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 409600,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Reshape_129_dma_init_out_0_133, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 409600 */

    static const LL_Switch_InitTypeDef switch_init_in_133[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_129 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=133 */
    LL_Switch_Init(switch_init_in_133, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_133_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_133_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x40);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_133[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_129 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=133 */
    LL_Switch_Deinit(switch_deinit_in_133, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_133_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_133_all_units, 2);

  }
  ec_trace_end_epoch(133);
  ec_trace_start_epoch(134);
  {
  }
  {
  }
  ec_trace_end_epoch(134);
  ec_trace_end_blob("_ec_blob_network_133");
}

void trace_ec__ec_blob_network_137(void) {
  ec_trace_start_blob("_ec_blob_network_137");
  ec_trace_start_epoch(137);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id785 */
    /* node=Identity_inserted_id785 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id784 */
    /* node=Identity_inserted_id784 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_131 */
    /* node=Reshape_131 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id785 input ports=0 range=1[307200,358400] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id785_dma_init_in_0_137 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Gemm_134_gemm_7_0_reshape_x_33_inserted_in785 */
      .offset_start = 307200,
      .offset_end = 308800,
      .offset_limit = 358464,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 32,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(0, &Identity_inserted_id785_dma_init_in_0_137, 1);

    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id784 input ports=0 range=1[358400,409600] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id784_dma_init_in_0_137 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Gemm_134_gemm_11_1_reshape_x_40_inserted_in784 */
      .offset_start = 358400,
      .offset_end = 360000,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 32,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Identity_inserted_id784_dma_init_in_0_137, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_131 input ports=0 range=2[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_131_dma_init_in_0_137 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Split_130_out_2 */
      .offset_start = 204800,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 64,
      .batch_depth = 2,
      .batch_offset = 8,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 204800,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Reshape_131_dma_init_in_0_137, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 102400 */
    /* npuRAM4 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id785 output ports=0 range=0[358400,409600] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id785_dma_init_out_0_137 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34350000UL) /* Equivalent hex address = 0x34350000UL */}, /* Gemm_134_gemm_7_0_reshape_x_33_inserted_out785 */
      .offset_start = 358400,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 128,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 51200,
      .frame_loop_cnt = 32,
      .frame_tot_cnt = 32,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Identity_inserted_id785_dma_init_out_0_137, 1);

    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id784 output ports=0 range=0[307200,358400] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id784_dma_init_out_0_137 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34350000UL) /* Equivalent hex address = 0x34350000UL */}, /* Gemm_134_gemm_11_1_reshape_x_40_inserted_out784 */
      .offset_start = 307200,
      .offset_limit = 358464,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 128,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 51200,
      .frame_loop_cnt = 32,
      .frame_tot_cnt = 32,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Identity_inserted_id784_dma_init_out_0_137, 1);

    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_131 output ports=0 range=1[0,204800] */

    static const LL_Streng_TensorInitTypeDef Reshape_131_dma_init_out_0_137 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_131_out_0 */
      .offset_start = 0,
      .offset_end = 1600,
      .offset_limit = 204864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 128,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Reshape_131_dma_init_out_0_137, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM6 <- 102400 */
    /* npuRAM5 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_137[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id785 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id784 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_131 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=137 */
    LL_Switch_Init(switch_init_in_137, 3);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_137_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_137_all_units, 6);

  }

  ec_trace_wait_epoch_end(0x310);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_137[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id785 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id784 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_131 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=137 */
    LL_Switch_Deinit(switch_deinit_in_137, 3);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_137_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_137_all_units, 6);

  }
  ec_trace_end_epoch(137);
  ec_trace_end_blob("_ec_blob_network_137");
}

void trace_ec__ec_blob_network_142(void) {
  ec_trace_start_blob("_ec_blob_network_142");
  ec_trace_start_epoch(142);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_134_out_0_0_10 */
    /* node=Gemm_134_out_0_0_10 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_134_out_0_0_14 */
    /* node=Gemm_134_out_0_0_14 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_10 input ports=0 range=11[1253376,1893376] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_10_dma_init_in_0_142 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* ???_627 */
      .offset_start = 1253376,
      .offset_limit = 1893440,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 1600,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 640000,
      .frame_loop_cnt = 400,
      .frame_tot_cnt = 400,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Gemm_134_out_0_0_10_dma_init_in_0_142, 1);

    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_14 input ports=0 range=11[0,640000] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_14_dma_init_in_0_142 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* ???_635 */
      .offset_start = 0,
      .offset_limit = 640064,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 1600,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 640000,
      .frame_loop_cnt = 400,
      .frame_tot_cnt = 400,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Gemm_134_out_0_0_14_dma_init_in_0_142, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 386048 */
    /* npuRAM3 -> 253952 */
    /* cpuRAM2 -> 640000 */

    /* Dma output units from cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_10 output ports=0 range=7[0,640000] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_10_dma_init_out_0_142 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_134_out_0_0_10_o */
      .offset_start = 0,
      .offset_end = 640000,
      .offset_limit = 640064,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 640000,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Gemm_134_out_0_0_10_dma_init_out_0_142, 1);

    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_14 output ports=0 range=7[640000,1280000] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_14_dma_init_out_0_142 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_134_out_0_0_14_o */
      .offset_start = 640000,
      .offset_end = 1280000,
      .offset_limit = 1280064,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 640000,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(0, &Gemm_134_out_0_0_14_dma_init_out_0_142, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 1280000 */

    static const LL_Switch_InitTypeDef switch_init_in_142[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_10 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_14 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=142 */
    LL_Switch_Init(switch_init_in_142, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_142_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_142_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x9);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_142[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_10 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_14 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=142 */
    LL_Switch_Deinit(switch_deinit_in_142, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_142_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_142_all_units, 4);

  }
  ec_trace_end_epoch(142);
  ec_trace_start_epoch(143);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id781 */
    /* node=Identity_inserted_id781 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id782 */
    /* node=Identity_inserted_id782 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id783 */
    /* node=Identity_inserted_id783 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id781 input ports=0 range=1[134400,204800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id781_dma_init_in_0_143 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_131_out_0_split_grp_X_75_127_o_2_inserted_in781 */
      .offset_start = 134400,
      .offset_end = 136000,
      .offset_limit = 204864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 44,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Identity_inserted_id781_dma_init_in_0_143, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id782 input ports=0 range=1[67200,134400] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id782_dma_init_in_0_143 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_131_out_0_split_grp_X_75_127_o_1_inserted_in782 */
      .offset_start = 67200,
      .offset_end = 68800,
      .offset_limit = 134464,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 42,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Identity_inserted_id782_dma_init_in_0_143, 1);

    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id783 input ports=0 range=1[0,67200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id783_dma_init_in_0_143 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_131_out_0_split_grp_X_75_127_o_0_inserted_in783 */
      .offset_start = 0,
      .offset_end = 1600,
      .offset_limit = 67264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 42,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Identity_inserted_id783_dma_init_in_0_143, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id781 output ports=0 range=2[70400,140800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id781_dma_init_out_0_143 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_131_out_0_split_grp_X_75_127_o_2_inserted_out781 */
      .offset_start = 70400,
      .offset_limit = 140864,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 176,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 70400,
      .frame_loop_cnt = 44,
      .frame_tot_cnt = 44,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Identity_inserted_id781_dma_init_out_0_143, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id782 output ports=0 range=2[140800,208000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id782_dma_init_out_0_143 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_131_out_0_split_grp_X_75_127_o_1_inserted_out782 */
      .offset_start = 140800,
      .offset_limit = 208064,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 168,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 67200,
      .frame_loop_cnt = 42,
      .frame_tot_cnt = 42,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Identity_inserted_id782_dma_init_out_0_143, 1);

    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id783 output ports=0 range=2[208000,275200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id783_dma_init_out_0_143 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_131_out_0_split_grp_X_75_127_o_0_inserted_out783 */
      .offset_start = 208000,
      .offset_limit = 275264,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 168,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 67200,
      .frame_loop_cnt = 42,
      .frame_tot_cnt = 42,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Identity_inserted_id783_dma_init_out_0_143, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_143[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id781 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id782 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id783 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=143 */
    LL_Switch_Init(switch_init_in_143, 3);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_143_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_143_all_units, 6);

  }

  ec_trace_wait_epoch_end(0x222);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_143[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id781 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id782 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id783 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=143 */
    LL_Switch_Deinit(switch_deinit_in_143, 3);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_143_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_143_all_units, 6);

  }
  ec_trace_end_epoch(143);
  ec_trace_end_blob("_ec_blob_network_142");
}

void trace_ec__ec_blob_network_148(void) {
  ec_trace_start_blob("_ec_blob_network_148");
  ec_trace_start_epoch(148);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id786 */
    /* node=Identity_inserted_id786 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id786 input ports=0 range=7[0,1280000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id786_dma_init_in_0_148 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_134_out_0_0_15_o_inserted_in786 */
      .offset_start = 0,
      .offset_end = 640000,
      .offset_limit = 1280064,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 640000,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Identity_inserted_id786_dma_init_in_0_148, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1280000 */

    /* Dma output units from cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id786 output ports=0 range=7[1280000,2560000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id786_dma_init_out_0_148 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_134_out_0_0_15_o_inserted_out786 */
      .offset_start = 1280000,
      .offset_limit = 2560064,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 400,
      .batch_depth = 2,
      .batch_offset = 8,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 1280000,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Identity_inserted_id786_dma_init_out_0_148, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 1280000 */

    static const LL_Switch_InitTypeDef switch_init_in_148[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id786 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=148 */
    LL_Switch_Init(switch_init_in_148, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_148_all_units[] = {
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_148_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x20);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_148[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id786 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=148 */
    LL_Switch_Deinit(switch_deinit_in_148, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_148_all_units[] = {
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_148_all_units, 2);

  }
  ec_trace_end_epoch(148);
  ec_trace_end_blob("_ec_blob_network_148");
}

void trace_ec__ec_blob_network_152(void) {
  ec_trace_start_blob("_ec_blob_network_152");
  ec_trace_start_epoch(152);
  {
  }
  {
  }
  ec_trace_end_epoch(152);
  ec_trace_end_blob("_ec_blob_network_152");
}

void trace_ec__ec_blob_network_154(void) {
  ec_trace_start_blob("_ec_blob_network_154");
  ec_trace_start_epoch(154);
  {
  }
  {
  }
  ec_trace_end_epoch(154);
  ec_trace_start_epoch(155);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id788 */
    /* node=Identity_inserted_id788 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id787 */
    /* node=Identity_inserted_id787 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id788 input ports=0 range=11[1253376,1893376] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id788_dma_init_in_0_155 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Gemm_138_gemm_23_0_reshape_x_47_inserted_in788 */
      .offset_start = 1253376,
      .offset_end = 1254976,
      .offset_limit = 1893440,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 400,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Identity_inserted_id788_dma_init_in_0_155, 1);

    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id787 input ports=0 range=11[0,640000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id787_dma_init_in_0_155 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Gemm_138_gemm_27_1_reshape_x_54_inserted_in787 */
      .offset_start = 0,
      .offset_end = 1600,
      .offset_limit = 640064,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 400,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Identity_inserted_id787_dma_init_in_0_155, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 386048 */
    /* npuRAM3 -> 253952 */
    /* cpuRAM2 -> 640000 */

    /* Dma output units from cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id788 output ports=0 range=7[0,640000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id788_dma_init_out_0_155 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_138_gemm_23_0_reshape_x_47_inserted_out788 */
      .offset_start = 0,
      .offset_limit = 640064,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 1600,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 640000,
      .frame_loop_cnt = 400,
      .frame_tot_cnt = 400,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Identity_inserted_id788_dma_init_out_0_155, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id787 output ports=0 range=7[640000,1280000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id787_dma_init_out_0_155 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_138_gemm_27_1_reshape_x_54_inserted_out787 */
      .offset_start = 640000,
      .offset_limit = 1280064,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 1600,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 640000,
      .frame_loop_cnt = 400,
      .frame_tot_cnt = 400,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Identity_inserted_id787_dma_init_out_0_155, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 1280000 */

    static const LL_Switch_InitTypeDef switch_init_in_155[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id788 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id787 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=155 */
    LL_Switch_Init(switch_init_in_155, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_155_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_155_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x22);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_155[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id788 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id787 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=155 */
    LL_Switch_Deinit(switch_deinit_in_155, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_155_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_155_all_units, 4);

  }
  ec_trace_end_epoch(155);
  ec_trace_end_blob("_ec_blob_network_154");
}

void trace_ec__ec_blob_network_158(void) {
  ec_trace_start_blob("_ec_blob_network_158");
  ec_trace_start_epoch(158);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_138_out_0_16_26 */
    /* node=Gemm_138_out_0_16_26 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_138_out_0_16_30 */
    /* node=Gemm_138_out_0_16_30 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_26 input ports=0 range=2[0,102400] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_26_dma_init_in_0_158 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* ???_656 */
      .offset_start = 0,
      .offset_limit = 102464,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 256,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 102400,
      .frame_loop_cnt = 64,
      .frame_tot_cnt = 64,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Gemm_138_out_0_16_26_dma_init_in_0_158, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_30 input ports=0 range=2[102400,204800] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_30_dma_init_in_0_158 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* ???_664 */
      .offset_start = 102400,
      .offset_limit = 204864,
      .frame_count = 0,
      .fwidth = 400,
      .fheight = 1,
      .batch_depth = 2,
      .batch_offset = 256,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 102400,
      .frame_loop_cnt = 64,
      .frame_tot_cnt = 64,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Gemm_138_out_0_16_30_dma_init_in_0_158, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_26 output ports=0 range=1[204800,307200] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_26_dma_init_out_0_158 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Gemm_138_out_0_16_26_o */
      .offset_start = 204800,
      .offset_end = 307200,
      .offset_limit = 307264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 102400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Gemm_138_out_0_16_26_dma_init_out_0_158, 1);

    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_30 output ports=0 range=1[307200,409600] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_30_dma_init_out_0_158 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Gemm_138_out_0_16_30_o */
      .offset_start = 307200,
      .offset_end = 409600,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 102400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(7, &Gemm_138_out_0_16_30_dma_init_out_0_158, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_158[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_26 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_30 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=158 */
    LL_Switch_Init(switch_init_in_158, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_158_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_158_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x90);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_158[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_26 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_30 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=158 */
    LL_Switch_Deinit(switch_deinit_in_158, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_158_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_158_all_units, 4);

  }
  ec_trace_end_epoch(158);
  ec_trace_start_epoch(159);
  {
  }
  {
  }
  ec_trace_end_epoch(159);
  ec_trace_start_epoch(160);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id789 */
    /* node=Identity_inserted_id789 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id789 input ports=0 range=1[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id789_dma_init_in_0_160 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_139_out_0_inserted_in789 */
      .offset_start = 204800,
      .offset_end = 206400,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 128,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Identity_inserted_id789_dma_init_in_0_160, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id789 output ports=0 range=2[0,204800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id789_dma_init_out_0_160 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_139_out_0_inserted_out789 */
      .offset_start = 0,
      .offset_limit = 204864,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 512,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 204800,
      .frame_loop_cnt = 128,
      .frame_tot_cnt = 128,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Identity_inserted_id789_dma_init_out_0_160, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_160[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id789 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=160 */
    LL_Switch_Init(switch_init_in_160, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_160_all_units[] = {
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_160_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x100);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_160[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id789 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=160 */
    LL_Switch_Deinit(switch_deinit_in_160, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_160_all_units[] = {
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_160_all_units, 2);

  }
  ec_trace_end_epoch(160);
  ec_trace_end_blob("_ec_blob_network_158");
}

void trace_ec__ec_blob_network_169(void) {
  ec_trace_start_blob("_ec_blob_network_169");
  ec_trace_start_epoch(169);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_148 */
    /* node=Concat_148 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_148 input ports=0 range=3[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_148_dma_init_in_0_169 = {
      /* from memory with batch=128 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34200000UL) /* Equivalent hex address = 0x34200000UL */}, /* Split_127_out_0 */
      .offset_start = 0,
      .offset_end = 204800,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 204800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_148_dma_init_in_0_169, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM3 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_148 output ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_148_dma_init_out_0_169 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Concat_148_out_0 */
      .offset_start = 0,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 256,
      .batch_offset = 1024,
      .frame_offset = 512,
      .line_offset = 0,
      .loop_offset = 409600,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(7, &Concat_148_dma_init_out_0_169, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 409600 */

    static const LL_Switch_InitTypeDef switch_init_in_169[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_148 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=169 */
    LL_Switch_Init(switch_init_in_169, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_169_all_units[] = {
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_169_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x80);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_169[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_148 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=169 */
    LL_Switch_Deinit(switch_deinit_in_169, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_169_all_units[] = {
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_169_all_units, 2);

  }
  ec_trace_end_epoch(169);
  ec_trace_end_blob("_ec_blob_network_169");
}

void trace_ec__ec_blob_network_173(void) {
  ec_trace_start_blob("_ec_blob_network_173");
  ec_trace_start_epoch(173);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Resize_152_resize_NN_expansion_concat_223 */
    /* node=Resize_152_resize_NN_expansion_concat_223 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_152_resize_NN_expansion_concat_223 input ports=0 range=7[13926400,14336000] */

    static const LL_Streng_TensorInitTypeDef Resize_152_resize_NN_expansion_concat_223_dma_init_in_0_173 = {
      /* from memory with batch=256
iterating outer iter=0 num_higher_elem=4
spanning across 1638400 bytes */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Mul_151_out_0 */
      .offset_start = 13926400,
      .offset_end = 14336000,
      .offset_limit = 14336064,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 409600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 1,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Resize_152_resize_NN_expansion_concat_223_dma_init_in_0_173, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1638400 */

    /* Dma output units from cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_152_resize_NN_expansion_concat_223 output ports=0 range=11[0,1638400] */

    static const LL_Streng_TensorInitTypeDef Resize_152_resize_NN_expansion_concat_223_dma_init_out_0_173 = {
      /* to memory canonical from batch=256 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Resize_152_resize_NN_expansion_concat_223_out_224 */
      .offset_start = 0,
      .offset_limit = 1638464,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 512,
      .batch_offset = 4096,
      .frame_offset = 1024,
      .line_offset = 0,
      .loop_offset = 1638400,
      .frame_loop_cnt = 4,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Resize_152_resize_NN_expansion_concat_223_dma_init_out_0_173, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 131072 */
    /* npuRAM3 <- 458752 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_173[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_152_resize_NN_expansion_concat_223 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=173 */
    LL_Switch_Init(switch_init_in_173, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_173_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_173_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x8);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_173[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_152_resize_NN_expansion_concat_223 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=173 */
    LL_Switch_Deinit(switch_deinit_in_173, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_173_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_173_all_units, 2);

  }
  ec_trace_end_epoch(173);
  ec_trace_end_blob("_ec_blob_network_173");
}

void trace_ec__ec_blob_network_187(void) {
  ec_trace_start_blob("_ec_blob_network_187");
  ec_trace_start_epoch(187);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_165 */
    /* node=Concat_165 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_165 input ports=0 range=7[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_165_dma_init_in_0_187 = {
      /* from memory with batch=64 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_157_out_0 */
      .offset_start = 0,
      .offset_end = 409600,
      .offset_limit = 1228864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 409600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Concat_165_dma_init_in_0_187, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1228800 */

    /* Dma output units from cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_165 output ports=0 range=11[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_165_dma_init_out_0_187 = {
      /* to memory canonical from batch=64 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_165_out_0 */
      .offset_start = 0,
      .offset_limit = 1228864,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 128,
      .batch_offset = 768,
      .frame_offset = 256,
      .line_offset = 0,
      .loop_offset = 1228800,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_165_dma_init_out_0_187, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM3 <- 180224 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_187[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_165 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=187 */
    LL_Switch_Init(switch_init_in_187, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_187_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_187_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x10);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_187[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_165 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=187 */
    LL_Switch_Deinit(switch_deinit_in_187, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_187_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_187_all_units, 2);

  }
  ec_trace_end_epoch(187);
  ec_trace_end_blob("_ec_blob_network_187");
}

void trace_ec__ec_blob_network_191(void) {
  ec_trace_start_blob("_ec_blob_network_191");
  ec_trace_start_epoch(191);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Resize_169_resize_NN_expansion_concat_227 */
    /* node=Resize_169_resize_NN_expansion_concat_227 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_169_resize_NN_expansion_concat_227 input ports=0 range=7[13107200,13926400] */

    static const LL_Streng_TensorInitTypeDef Resize_169_resize_NN_expansion_concat_227_dma_init_in_0_191 = {
      /* from memory with batch=128
iterating outer iter=0 num_higher_elem=4
spanning across 3276800 bytes */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Mul_168_out_0 */
      .offset_start = 13107200,
      .offset_end = 13926400,
      .offset_limit = 13926464,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 819200,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 1,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Resize_169_resize_NN_expansion_concat_227_dma_init_in_0_191, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 3276800 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_169_resize_NN_expansion_concat_227 output ports=0 range=7[0,3276800] */

    static const LL_Streng_TensorInitTypeDef Resize_169_resize_NN_expansion_concat_227_dma_init_out_0_191 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Resize_169_resize_NN_expansion_concat_227_out_228 */
      .offset_start = 0,
      .offset_limit = 3276864,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 256,
      .batch_offset = 2048,
      .frame_offset = 512,
      .line_offset = 0,
      .loop_offset = 3276800,
      .frame_loop_cnt = 4,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Resize_169_resize_NN_expansion_concat_227_dma_init_out_0_191, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 3276800 */

    static const LL_Switch_InitTypeDef switch_init_in_191[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_169_resize_NN_expansion_concat_227 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=191 */
    LL_Switch_Init(switch_init_in_191, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_191_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_191_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x4);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_191[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_169_resize_NN_expansion_concat_227 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=191 */
    LL_Switch_Deinit(switch_deinit_in_191, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_191_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_191_all_units, 2);

  }
  ec_trace_end_epoch(191);
  ec_trace_end_blob("_ec_blob_network_191");
}

void trace_ec__ec_blob_network_193(void) {
  ec_trace_start_blob("_ec_blob_network_193");
  ec_trace_start_epoch(193);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_170 */
    /* node=Concat_170 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_170 input ports=0 range=7[6553600,13107200] */

    static const LL_Streng_TensorInitTypeDef Concat_170_dma_init_in_0_193 = {
      /* from memory with batch=128 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Resize_169_resize_NN_expansion_concat_227_out_230 */
      .offset_start = 6553600,
      .offset_end = 9830400,
      .offset_limit = 13107264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 3276800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Concat_170_dma_init_in_0_193, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 6553600 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_170 output ports=0 range=7[0,6553600] */

    static const LL_Streng_TensorInitTypeDef Concat_170_dma_init_out_0_193 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Concat_170_out_0 */
      .offset_start = 0,
      .offset_limit = 6553664,
      .frame_count = 0,
      .fwidth = 80,
      .fheight = 80,
      .batch_depth = 256,
      .batch_offset = 1024,
      .frame_offset = 512,
      .line_offset = 0,
      .loop_offset = 6553600,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Concat_170_dma_init_out_0_193, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 6553600 */

    static const LL_Switch_InitTypeDef switch_init_in_193[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_170 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=193 */
    LL_Switch_Init(switch_init_in_193, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_193_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_193_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x200);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_193[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_170 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=193 */
    LL_Switch_Deinit(switch_deinit_in_193, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_193_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_193_all_units, 2);

  }
  ec_trace_end_epoch(193);
  ec_trace_end_blob("_ec_blob_network_193");
}

void trace_ec__ec_blob_network_205(void) {
  ec_trace_start_blob("_ec_blob_network_205");
  ec_trace_start_epoch(205);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_182 */
    /* node=Concat_182 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_182 input ports=0 range=7[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_182_dma_init_in_0_205 = {
      /* from memory with batch=32 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_174_out_0 */
      .offset_start = 0,
      .offset_end = 819200,
      .offset_limit = 2457664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 819200,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Concat_182_dma_init_in_0_205, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2457600 */

    /* Dma output units from cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_182 output ports=0 range=11[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_182_dma_init_out_0_205 = {
      /* to memory canonical from batch=32 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_182_out_0 */
      .offset_start = 0,
      .offset_limit = 2457664,
      .frame_count = 0,
      .fwidth = 80,
      .fheight = 80,
      .batch_depth = 64,
      .batch_offset = 384,
      .frame_offset = 128,
      .line_offset = 0,
      .loop_offset = 2457600,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_182_dma_init_out_0_205, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM6 <- 32768 */
    /* npuRAM5 <- 458752 */
    /* npuRAM4 <- 458752 */
    /* npuRAM3 <- 458752 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_205[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_182 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=205 */
    LL_Switch_Init(switch_init_in_205, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_205_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_205_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x10);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_205[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_182 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=205 */
    LL_Switch_Deinit(switch_deinit_in_205, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_205_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_205_all_units, 2);

  }
  ec_trace_end_epoch(205);
  ec_trace_end_blob("_ec_blob_network_205");
}

void trace_ec__ec_blob_network_257(void) {
  ec_trace_start_blob("_ec_blob_network_257");
  ec_trace_start_epoch(257);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_231 */
    /* node=Concat_231 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_215 */
    /* node=Reshape_215 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_231 input ports=0 range=7[5427200,6656000] */

    static const LL_Streng_TensorInitTypeDef Concat_231_dma_init_in_0_257 = {
      /* from memory with batch=64 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_223_out_0 */
      .offset_start = 5427200,
      .offset_end = 5836800,
      .offset_limit = 6656064,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 409600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Concat_231_dma_init_in_0_257, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_215 input ports=0 range=7[7065600,8704000] */

    static const LL_Streng_TensorInitTypeDef Reshape_215_dma_init_in_0_257 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Conv2D_214_out_0 */
      .offset_start = 7065600,
      .offset_limit = 8704064,
      .frame_count = 0,
      .fwidth = 80,
      .fheight = 80,
      .batch_depth = 2,
      .batch_offset = 256,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 1638400,
      .frame_loop_cnt = 64,
      .frame_tot_cnt = 64,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Reshape_215_dma_init_in_0_257, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2867200 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_231 output ports=0 range=11[1483776,2712576] */

    static const LL_Streng_TensorInitTypeDef Concat_231_dma_init_out_0_257 = {
      /* to memory canonical from batch=64 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_231_out_0 */
      .offset_start = 1483776,
      .offset_limit = 2712640,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 128,
      .batch_offset = 768,
      .frame_offset = 256,
      .line_offset = 0,
      .loop_offset = 1228800,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Concat_231_dma_init_out_0_257, 1);

    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_215 output ports=0 range=7[2150400,3788800] */

    static const LL_Streng_TensorInitTypeDef Reshape_215_dma_init_out_0_257 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Reshape_215_out_0 */
      .offset_start = 2150400,
      .offset_end = 3788800,
      .offset_limit = 3788864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 1638400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Reshape_215_dma_init_out_0_257, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM6 <- 287744 */
    /* npuRAM5 <- 458752 */
    /* npuRAM4 <- 458752 */
    /* npuRAM3 <- 23552 */
    /* hyperRAM <- 1638400 */

    static const LL_Switch_InitTypeDef switch_init_in_257[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_231 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_215 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=257 */
    LL_Switch_Init(switch_init_in_257, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_257_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_257_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x202);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_257[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_231 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_215 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=257 */
    LL_Switch_Deinit(switch_deinit_in_257, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_257_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_257_all_units, 4);

  }
  ec_trace_end_epoch(257);
  ec_trace_end_blob("_ec_blob_network_257");
}

void trace_ec__ec_blob_network_326(void) {
  ec_trace_start_blob("_ec_blob_network_326");
  ec_trace_start_epoch(326);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_293 */
    /* node=Concat_293 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_264 */
    /* node=Reshape_264 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_293 input ports=0 range=2[0,204800] */

    static const LL_Streng_TensorInitTypeDef Concat_293_dma_init_in_0_326 = {
      /* from memory with batch=64 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Add_292_out_0 */
      .offset_start = 0,
      .offset_end = 102400,
      .offset_limit = 204864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 102400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(7, &Concat_293_dma_init_in_0_326, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_264 input ports=0 range=11[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_264_dma_init_in_0_326 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Conv2D_263_out_0 */
      .offset_start = 0,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 2,
      .batch_offset = 256,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 409600,
      .frame_loop_cnt = 64,
      .frame_tot_cnt = 64,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Reshape_264_dma_init_in_0_326, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 204800 */
    /* cpuRAM2 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_293 output ports=0 range=2[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_293_dma_init_out_0_326 = {
      /* to memory canonical from batch=64 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Concat_293_out_0 */
      .offset_start = 204800,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 128,
      .batch_offset = 512,
      .frame_offset = 256,
      .line_offset = 0,
      .loop_offset = 204800,
      .frame_loop_cnt = 2,
      .frame_tot_cnt = 2,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Concat_293_dma_init_out_0_326, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_264 output ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_264_dma_init_out_0_326 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_264_out_0 */
      .offset_start = 0,
      .offset_end = 409600,
      .offset_limit = 409664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 409600,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Reshape_264_dma_init_out_0_326, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 409600 */
    /* npuRAM4 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_326[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_293 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_264 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=326 */
    LL_Switch_Init(switch_init_in_326, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_326_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_326_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x12);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_326[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_293 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_264 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=326 */
    LL_Switch_Deinit(switch_deinit_in_326, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_326_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_326_all_units, 4);

  }
  ec_trace_end_epoch(326);
  ec_trace_end_blob("_ec_blob_network_326");
}

void trace_ec__ec_blob_network_334(void) {
  ec_trace_start_blob("_ec_blob_network_334");
  ec_trace_start_epoch(334);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_297 */
    /* node=Concat_297 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_297 input ports=0 range=7[0,614400] */

    static const LL_Streng_TensorInitTypeDef Concat_297_dma_init_in_0_334 = {
      /* from memory with batch=128 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_272_out_0 */
      .offset_start = 0,
      .offset_end = 204800,
      .offset_limit = 614464,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 204800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Concat_297_dma_init_in_0_334, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 614400 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_297 output ports=0 range=11[0,614400] */

    static const LL_Streng_TensorInitTypeDef Concat_297_dma_init_out_0_334 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_297_out_0 */
      .offset_start = 0,
      .offset_limit = 614464,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 256,
      .batch_offset = 1536,
      .frame_offset = 512,
      .line_offset = 0,
      .loop_offset = 614400,
      .frame_loop_cnt = 3,
      .frame_tot_cnt = 3,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Concat_297_dma_init_out_0_334, 1);


    /* Dma output bandwidth to memory pools: */
    /* cpuRAM2 <- 614400 */

    static const LL_Switch_InitTypeDef switch_init_in_334[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_297 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=334 */
    LL_Switch_Init(switch_init_in_334, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_334_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_334_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_334[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_297 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=334 */
    LL_Switch_Deinit(switch_deinit_in_334, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_334_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_334_all_units, 2);

  }
  ec_trace_end_epoch(334);
  ec_trace_end_blob("_ec_blob_network_334");
}

void trace_ec__ec_blob_network_371(void) {
  ec_trace_start_blob("_ec_blob_network_371");
  ec_trace_start_epoch(371);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_342 */
    /* node=Reshape_342 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_242 */
    /* node=Reshape_242 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_193 */
    /* node=Reshape_193 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_342 input ports=0 range=11[0,102400] */

    static const LL_Streng_TensorInitTypeDef Reshape_342_dma_init_in_0_371 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Conv2D_341_out_0 */
      .offset_start = 0,
      .offset_limit = 102464,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 256,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 102400,
      .frame_loop_cnt = 64,
      .frame_tot_cnt = 64,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(4, &Reshape_342_dma_init_in_0_371, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_242 input ports=0 range=7[1075200,1190400] */

    static const LL_Streng_TensorInitTypeDef Reshape_242_dma_init_in_0_371 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 1,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Conv2D_241_concat_247_Out */
      .offset_start = 1075200,
      .offset_limit = 1190464,
      .frame_count = 0,
      .fwidth = 40,
      .fheight = 40,
      .batch_depth = 2,
      .batch_offset = 72,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 115200,
      .frame_loop_cnt = 18,
      .frame_tot_cnt = 18,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(1, &Reshape_242_dma_init_in_0_371, 1);

    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_193 input ports=0 range=7[614400,1075200] */

    static const LL_Streng_TensorInitTypeDef Reshape_193_dma_init_in_0_371 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Conv2D_192_concat_237_Out */
      .offset_start = 614400,
      .offset_limit = 1075264,
      .frame_count = 0,
      .fwidth = 80,
      .fheight = 80,
      .batch_depth = 2,
      .batch_offset = 72,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 460800,
      .frame_loop_cnt = 18,
      .frame_tot_cnt = 18,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Reshape_193_dma_init_in_0_371, 1);


    /* Dma input bandwidth from memory pools: */
    /* cpuRAM2 -> 102400 */
    /* hyperRAM -> 576000 */
    /* CACHE -> 0 */

    /* Dma output units from cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_342 output ports=0 range=2[115200,217600] */

    static const LL_Streng_TensorInitTypeDef Reshape_342_dma_init_out_0_371 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_342_out_0 */
      .offset_start = 115200,
      .offset_end = 217600,
      .offset_limit = 217664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 102400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Reshape_342_dma_init_out_0_371, 1);

    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_242 output ports=0 range=2[0,115200] */

    static const LL_Streng_TensorInitTypeDef Reshape_242_dma_init_out_0_371 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_242_out_0 */
      .offset_start = 0,
      .offset_end = 115200,
      .offset_limit = 115264,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 115200,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(0, &Reshape_242_dma_init_out_0_371, 1);

    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_193 output ports=0 range=11[604800,1065600] */

    static const LL_Streng_TensorInitTypeDef Reshape_193_dma_init_out_0_371 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Reshape_193_out_0 */
      .offset_start = 604800,
      .offset_end = 1065600,
      .offset_limit = 1065664,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 460800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Reshape_193_dma_init_out_0_371, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 217600 */
    /* npuRAM3 <- 17024 */
    /* cpuRAM2 <- 443776 */

    static const LL_Switch_InitTypeDef switch_init_in_371[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_342 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_242 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_193 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=371 */
    LL_Switch_Init(switch_init_in_371, 3);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_371_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_371_all_units, 6);

  }

  ec_trace_wait_epoch_end(0x301);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_371[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_342 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_242 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_193 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=371 */
    LL_Switch_Deinit(switch_deinit_in_371, 3);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_371_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_371_all_units, 6);

  }
  ec_trace_end_epoch(371);
  ec_trace_end_blob("_ec_blob_network_371");
}

void trace_ec__ec_blob_network_378(void) {
  ec_trace_start_blob("_ec_blob_network_378");
  ec_trace_start_epoch(378);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id790 */
    /* node=Identity_inserted_id790 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id790 input ports=0 range=7[2150400,4300800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id790_dma_init_in_0_378 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Transpose_345_out_0_inserted_in790 */
      .offset_start = 2150400,
      .offset_end = 2284800,
      .offset_limit = 4300864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 134400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 16,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(9, &Identity_inserted_id790_dma_init_in_0_378, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2150400 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id790 output ports=0 range=7[4300800,6451200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id790_dma_init_out_0_378 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Transpose_345_out_0_inserted_out790 */
      .offset_start = 4300800,
      .offset_limit = 6451264,
      .frame_count = 0,
      .fwidth = 8400,
      .fheight = 4,
      .batch_depth = 2,
      .batch_offset = 64,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 2150400,
      .frame_loop_cnt = 16,
      .frame_tot_cnt = 16,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Identity_inserted_id790_dma_init_out_0_378, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 2150400 */

    static const LL_Switch_InitTypeDef switch_init_in_378[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id790 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=378 */
    LL_Switch_Init(switch_init_in_378, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_378_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_378_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x4);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_378[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id790 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=378 */
    LL_Switch_Deinit(switch_deinit_in_378, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_378_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_378_all_units, 2);

  }
  ec_trace_end_epoch(378);
  ec_trace_end_blob("_ec_blob_network_378");
}

void trace_ec__ec_blob_network_380(void) {
  ec_trace_start_blob("_ec_blob_network_380");
  ec_trace_start_epoch(380);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_308 */
    /* node=Reshape_308 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_308 input ports=0 range=1[0,28800] */

    static const LL_Streng_TensorInitTypeDef Reshape_308_dma_init_in_0_380 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Conv2D_307_concat_257_Out */
      .offset_start = 0,
      .offset_limit = 28864,
      .frame_count = 0,
      .fwidth = 20,
      .fheight = 20,
      .batch_depth = 2,
      .batch_offset = 72,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 28800,
      .frame_loop_cnt = 18,
      .frame_tot_cnt = 18,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(8, &Reshape_308_dma_init_in_0_380, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 28800 */

    /* Dma output units from cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_308 output ports=0 range=1[201600,230400] */

    static const LL_Streng_TensorInitTypeDef Reshape_308_dma_init_out_0_380 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_308_out_0 */
      .offset_start = 201600,
      .offset_end = 230400,
      .offset_limit = 230464,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 28800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Reshape_308_dma_init_out_0_380, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 28800 */

    static const LL_Switch_InitTypeDef switch_init_in_380[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_308 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=380 */
    LL_Switch_Init(switch_init_in_380, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_380_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_380_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x8);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_380[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_308 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=380 */
    LL_Switch_Deinit(switch_deinit_in_380, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_380_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_380_all_units, 2);

  }
  ec_trace_end_epoch(380);
  ec_trace_end_blob("_ec_blob_network_380");
}

void trace_ec__ec_blob_network_394(void) {
  ec_trace_start_blob("_ec_blob_network_394");
  ec_trace_start_epoch(394);
  {
  }
  {
  }
  ec_trace_end_epoch(394);
  ec_trace_start_epoch(395);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Split node=Slice_350 */
    /* node=Slice_350 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id792 */
    /* node=Identity_inserted_id792 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Slice_350 input ports=0 range=1[0,134400] */

    static const LL_Streng_TensorInitTypeDef Slice_350_dma_init_in_0_395 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_348_out_0 */
      .offset_start = 0,
      .offset_end = 134400,
      .offset_limit = 134464,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 134400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Slice_350_dma_init_in_0_395, 1);

    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id792 input ports=0 range=11[0,604800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id792_dma_init_in_0_395 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Reshape_310_out_0_inserted_in792 */
      .offset_start = 0,
      .offset_end = 100800,
      .offset_limit = 604864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 100800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 6,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(3, &Identity_inserted_id792_dma_init_in_0_395, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 134400 */
    /* cpuRAM2 -> 604800 */

    /* Dma output units from cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Slice_350 output ports=0 range=1[134400,268800] */

    static const LL_Streng_TensorInitTypeDef Slice_350_dma_init_out_0_395 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Slice_350_out_0 */
      .offset_start = 134400,
      .offset_end = 268800,
      .offset_limit = 268864,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 134400,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(0, &Slice_350_dma_init_out_0_395, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id792 output ports=0 range=7[0,604800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id792_dma_init_out_0_395 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Reshape_310_out_0_inserted_out792 */
      .offset_start = 0,
      .offset_limit = 604864,
      .frame_count = 0,
      .fwidth = 8400,
      .fheight = 3,
      .batch_depth = 2,
      .batch_offset = 24,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 604800,
      .frame_loop_cnt = 6,
      .frame_tot_cnt = 6,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Identity_inserted_id792_dma_init_out_0_395, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 134400 */
    /* hyperRAM <- 604800 */

    static const LL_Switch_InitTypeDef switch_init_in_395[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Slice_350 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id792 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=395 */
    LL_Switch_Init(switch_init_in_395, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_395_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_395_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x41);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_395[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Slice_350 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id792 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=395 */
    LL_Switch_Deinit(switch_deinit_in_395, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_395_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_395_all_units, 4);

  }
  ec_trace_end_epoch(395);
  ec_trace_end_blob("_ec_blob_network_394");
}

void trace_ec__ec_blob_network_403(void) {
  ec_trace_start_blob("_ec_blob_network_403");
  ec_trace_start_epoch(403);
  {
  }
  {
  }
  ec_trace_end_epoch(403);
  ec_trace_end_blob("_ec_blob_network_403");
}

void trace_ec__ec_blob_network_409(void) {
  ec_trace_start_blob("_ec_blob_network_409");
  ec_trace_start_epoch(409);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_318 */
    /* node=Reshape_318 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_318 input ports=0 range=11[0,604800] */

    static const LL_Streng_TensorInitTypeDef Reshape_318_dma_init_in_0_409 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_317_out_0 */
      .offset_start = 0,
      .offset_limit = 604864,
      .frame_count = 0,
      .fwidth = 8400,
      .fheight = 3,
      .batch_depth = 2,
      .batch_offset = 24,
      .frame_offset = 4,
      .line_offset = 0,
      .loop_offset = 604800,
      .frame_loop_cnt = 6,
      .frame_tot_cnt = 6,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(2, &Reshape_318_dma_init_in_0_409, 1);


    /* Dma input bandwidth from memory pools: */
    /* cpuRAM2 -> 604800 */

    /* Dma output units from cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_318 output ports=0 range=11[1216576,1821376] */

    static const LL_Streng_TensorInitTypeDef Reshape_318_dma_init_out_0_409 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Reshape_318_out_0 */
      .offset_start = 1216576,
      .offset_end = 1821376,
      .offset_limit = 1821440,
      .frame_count = 0,
      .fwidth = 0,
      .fheight = 0,
      .batch_depth = 0,
      .batch_offset = 0,
      .frame_offset = 604800,
      .line_offset = 0,
      .loop_offset = 0,
      .frame_loop_cnt = 0,
      .frame_tot_cnt = 1,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(5, &Reshape_318_dma_init_out_0_409, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 314048 */
    /* npuRAM3 <- 290752 */

    static const LL_Switch_InitTypeDef switch_init_in_409[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_318 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=409 */
    LL_Switch_Init(switch_init_in_409, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_409_all_units[] = {
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_409_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x20);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_409[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_318 OUT: in unit=STREAM_ENG_V2 5 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=409 */
    LL_Switch_Deinit(switch_deinit_in_409, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_409_all_units[] = {
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_409_all_units, 2);

  }
  ec_trace_end_epoch(409);
  ec_trace_end_blob("_ec_blob_network_409");
}


int main () {
  ec_trace_init("network_ecblobs.h", "network", false);
  trace_ec__ec_blob_network_1();
  trace_ec__ec_blob_network_8();
  trace_ec__ec_blob_network_11();
  trace_ec__ec_blob_network_22();
  trace_ec__ec_blob_network_40();
  trace_ec__ec_blob_network_71();
  trace_ec__ec_blob_network_75();
  trace_ec__ec_blob_network_106();
  trace_ec__ec_blob_network_110();
  trace_ec__ec_blob_network_124();
  trace_ec__ec_blob_network_133();
  trace_ec__ec_blob_network_137();
  trace_ec__ec_blob_network_142();
  trace_ec__ec_blob_network_148();
  trace_ec__ec_blob_network_152();
  trace_ec__ec_blob_network_154();
  trace_ec__ec_blob_network_158();
  trace_ec__ec_blob_network_169();
  trace_ec__ec_blob_network_173();
  trace_ec__ec_blob_network_187();
  trace_ec__ec_blob_network_191();
  trace_ec__ec_blob_network_193();
  trace_ec__ec_blob_network_205();
  trace_ec__ec_blob_network_257();
  trace_ec__ec_blob_network_326();
  trace_ec__ec_blob_network_334();
  trace_ec__ec_blob_network_371();
  trace_ec__ec_blob_network_378();
  trace_ec__ec_blob_network_380();
  trace_ec__ec_blob_network_394();
  trace_ec__ec_blob_network_403();
  trace_ec__ec_blob_network_409();
  ec_trace_all_blobs_done();
}
