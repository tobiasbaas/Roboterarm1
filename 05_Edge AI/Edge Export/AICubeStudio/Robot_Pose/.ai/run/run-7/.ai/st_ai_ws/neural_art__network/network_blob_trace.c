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
    /* kind=Identity node=Identity_inserted_id665 */
    /* node=Identity_inserted_id665 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id665 input ports=0 range=7[0,4915200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id665_dma_init_in_0_1 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Input_6_out_0_inserted_in665 */
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
    LL_Streng_TensorInit(5, &Identity_inserted_id665_dma_init_in_0_1, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 4915200 */

    /* Dma output units from cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id665 output ports=0 range=7[4915200,9830400] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id665_dma_init_out_0_1 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Input_6_out_0_inserted_out665 */
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
    LL_Streng_TensorInit(3, &Identity_inserted_id665_dma_init_out_0_1, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 4915200 */

    static const LL_Switch_InitTypeDef switch_init_in_1[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id665 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=1 */
    LL_Switch_Init(switch_init_in_1, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_1_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_1_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x8);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_1[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id665 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=1 */
    LL_Switch_Deinit(switch_deinit_in_1, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_1_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_1_all_units, 2);

  }
  ec_trace_end_epoch(1);
  ec_trace_end_blob("_ec_blob_network_1");
}

void trace_ec__ec_blob_network_19(void) {
  ec_trace_start_blob("_ec_blob_network_19");
  ec_trace_start_epoch(19);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_24 */
    /* node=Concat_24 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_24 input ports=0 range=7[0,4915200] */

    static const LL_Streng_TensorInitTypeDef Concat_24_dma_init_in_0_19 = {
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
    LL_Streng_TensorInit(7, &Concat_24_dma_init_in_0_19, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 4915200 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_24 output ports=0 range=7[4915200,9830400] */

    static const LL_Streng_TensorInitTypeDef Concat_24_dma_init_out_0_19 = {
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
    LL_Streng_TensorInit(1, &Concat_24_dma_init_out_0_19, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 4915200 */

    static const LL_Switch_InitTypeDef switch_init_in_19[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_24 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=19 */
    LL_Switch_Init(switch_init_in_19, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_19_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_19_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_19[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_24 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=19 */
    LL_Switch_Deinit(switch_deinit_in_19, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_19_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_19_all_units, 2);

  }
  ec_trace_end_epoch(19);
  ec_trace_end_blob("_ec_blob_network_19");
}

void trace_ec__ec_blob_network_37(void) {
  ec_trace_start_blob("_ec_blob_network_37");
  ec_trace_start_epoch(37);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_42 */
    /* node=Concat_42 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_42 input ports=0 range=7[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_42_dma_init_in_0_37 = {
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
    LL_Streng_TensorInit(1, &Concat_42_dma_init_in_0_37, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2457600 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_42 output ports=0 range=11[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_42_dma_init_out_0_37 = {
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
    LL_Streng_TensorInit(6, &Concat_42_dma_init_out_0_37, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM6 <- 32768 */
    /* npuRAM5 <- 458752 */
    /* npuRAM4 <- 458752 */
    /* npuRAM3 <- 458752 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_37[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_42 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=37 */
    LL_Switch_Init(switch_init_in_37, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_37_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_37_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x40);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_37[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_42 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=37 */
    LL_Switch_Deinit(switch_deinit_in_37, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_37_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_37_all_units, 2);

  }
  ec_trace_end_epoch(37);
  ec_trace_end_blob("_ec_blob_network_37");
}

void trace_ec__ec_blob_network_68(void) {
  ec_trace_start_blob("_ec_blob_network_68");
  ec_trace_start_epoch(68);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_73 */
    /* node=Concat_73 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_73 input ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_73_dma_init_in_0_68 = {
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
    LL_Streng_TensorInit(5, &Concat_73_dma_init_in_0_68, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_73 output ports=0 range=2[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_73_dma_init_out_0_68 = {
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
    LL_Streng_TensorInit(0, &Concat_73_dma_init_out_0_68, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 409600 */

    static const LL_Switch_InitTypeDef switch_init_in_68[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_73 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=68 */
    LL_Switch_Init(switch_init_in_68, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_68_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_68_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x1);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_68[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_73 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=68 */
    LL_Switch_Deinit(switch_deinit_in_68, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_68_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_68_all_units, 2);

  }
  ec_trace_end_epoch(68);
  ec_trace_end_blob("_ec_blob_network_68");
}

void trace_ec__ec_blob_network_72(void) {
  ec_trace_start_blob("_ec_blob_network_72");
  ec_trace_start_epoch(72);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_77 */
    /* node=Concat_77 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_77 input ports=0 range=7[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_77_dma_init_in_0_72 = {
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
    LL_Streng_TensorInit(8, &Concat_77_dma_init_in_0_72, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1228800 */

    /* Dma output units from cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_77 output ports=0 range=11[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_77_dma_init_out_0_72 = {
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
    LL_Streng_TensorInit(4, &Concat_77_dma_init_out_0_72, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM3 <- 180224 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_72[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_77 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=72 */
    LL_Switch_Init(switch_init_in_72, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_72_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_72_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x10);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_72[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_77 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=72 */
    LL_Switch_Deinit(switch_deinit_in_72, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_72_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_72_all_units, 2);

  }
  ec_trace_end_epoch(72);
  ec_trace_end_blob("_ec_blob_network_72");
}

void trace_ec__ec_blob_network_103(void) {
  ec_trace_start_blob("_ec_blob_network_103");
  ec_trace_start_epoch(103);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_108 */
    /* node=Concat_108 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_108 input ports=0 range=1[0,204800] */

    static const LL_Streng_TensorInitTypeDef Concat_108_dma_init_in_0_103 = {
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
    LL_Streng_TensorInit(0, &Concat_108_dma_init_in_0_103, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_108 output ports=0 range=1[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_108_dma_init_out_0_103 = {
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
    LL_Streng_TensorInit(9, &Concat_108_dma_init_out_0_103, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_103[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_108 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=103 */
    LL_Switch_Init(switch_init_in_103, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_103_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_103_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x200);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_103[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_108 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=103 */
    LL_Switch_Deinit(switch_deinit_in_103, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_103_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_103_all_units, 2);

  }
  ec_trace_end_epoch(103);
  ec_trace_end_blob("_ec_blob_network_103");
}

void trace_ec__ec_blob_network_107(void) {
  ec_trace_start_blob("_ec_blob_network_107");
  ec_trace_start_epoch(107);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_112 */
    /* node=Concat_112 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_112 input ports=0 range=11[614400,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_112_dma_init_in_0_107 = {
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
    LL_Streng_TensorInit(2, &Concat_112_dma_init_in_0_107, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM3 -> 180224 */
    /* cpuRAM2 -> 434176 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_112 output ports=0 range=11[0,614400] */

    static const LL_Streng_TensorInitTypeDef Concat_112_dma_init_out_0_107 = {
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
    LL_Streng_TensorInit(9, &Concat_112_dma_init_out_0_107, 1);


    /* Dma output bandwidth to memory pools: */
    /* cpuRAM2 <- 614400 */

    static const LL_Switch_InitTypeDef switch_init_in_107[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_112 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=107 */
    LL_Switch_Init(switch_init_in_107, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_107_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_107_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x200);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_107[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_112 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=107 */
    LL_Switch_Deinit(switch_deinit_in_107, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_107_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_107_all_units, 2);

  }
  ec_trace_end_epoch(107);
  ec_trace_end_blob("_ec_blob_network_107");
}

void trace_ec__ec_blob_network_121(void) {
  ec_trace_start_blob("_ec_blob_network_121");
  ec_trace_start_epoch(121);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_120 */
    /* node=Concat_120 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_120 input ports=0 range=7[0,819200] */

    static const LL_Streng_TensorInitTypeDef Concat_120_dma_init_in_0_121 = {
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
    LL_Streng_TensorInit(0, &Concat_120_dma_init_in_0_121, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 819200 */

    /* Dma output units from cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_120 output ports=0 range=11[0,819200] */

    static const LL_Streng_TensorInitTypeDef Concat_120_dma_init_out_0_121 = {
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
    LL_Streng_TensorInit(7, &Concat_120_dma_init_out_0_121, 1);


    /* Dma output bandwidth to memory pools: */
    /* cpuRAM2 <- 819200 */

    static const LL_Switch_InitTypeDef switch_init_in_121[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_120 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=121 */
    LL_Switch_Init(switch_init_in_121, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_121_all_units[] = {
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_121_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x80);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_121[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_120 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=121 */
    LL_Switch_Deinit(switch_deinit_in_121, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_121_all_units[] = {
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_121_all_units, 2);

  }
  ec_trace_end_epoch(121);
  ec_trace_end_blob("_ec_blob_network_121");
}

void trace_ec__ec_blob_network_130(void) {
  ec_trace_start_blob("_ec_blob_network_130");
  ec_trace_start_epoch(130);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_129 */
    /* node=Reshape_129 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_129 input ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_129_dma_init_in_0_130 = {
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
    LL_Streng_TensorInit(7, &Reshape_129_dma_init_in_0_130, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_129 output ports=0 range=2[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_129_dma_init_out_0_130 = {
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
    LL_Streng_TensorInit(2, &Reshape_129_dma_init_out_0_130, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 409600 */

    static const LL_Switch_InitTypeDef switch_init_in_130[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_129 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=130 */
    LL_Switch_Init(switch_init_in_130, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_130_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_130_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x4);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_130[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_129 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=130 */
    LL_Switch_Deinit(switch_deinit_in_130, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_130_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_130_all_units, 2);

  }
  ec_trace_end_epoch(130);
  ec_trace_start_epoch(131);
  {
  }
  {
  }
  ec_trace_end_epoch(131);
  ec_trace_end_blob("_ec_blob_network_130");
}

void trace_ec__ec_blob_network_134(void) {
  ec_trace_start_blob("_ec_blob_network_134");
  ec_trace_start_epoch(134);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id667 */
    /* node=Identity_inserted_id667 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id666 */
    /* node=Identity_inserted_id666 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_131 */
    /* node=Reshape_131 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id667 input ports=0 range=1[102400,153600] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id667_dma_init_in_0_134 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Gemm_134_gemm_7_0_reshape_x_33_inserted_in667 */
      .offset_start = 102400,
      .offset_end = 104000,
      .offset_limit = 153664,
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
    LL_Streng_TensorInit(7, &Identity_inserted_id667_dma_init_in_0_134, 1);

    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id666 input ports=0 range=1[153600,204800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id666_dma_init_in_0_134 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Gemm_134_gemm_11_1_reshape_x_40_inserted_in666 */
      .offset_start = 153600,
      .offset_end = 155200,
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
      .frame_tot_cnt = 32,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(0, &Identity_inserted_id666_dma_init_in_0_134, 1);

    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_131 input ports=0 range=2[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_131_dma_init_in_0_134 = {
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
    LL_Streng_TensorInit(2, &Reshape_131_dma_init_in_0_134, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 102400 */
    /* npuRAM4 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id667 output ports=0 range=0[358400,409600] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id667_dma_init_out_0_134 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34350000UL) /* Equivalent hex address = 0x34350000UL */}, /* Gemm_134_gemm_7_0_reshape_x_33_inserted_out667 */
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
    LL_Streng_TensorInit(6, &Identity_inserted_id667_dma_init_out_0_134, 1);

    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id666 output ports=0 range=0[307200,358400] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id666_dma_init_out_0_134 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34350000UL) /* Equivalent hex address = 0x34350000UL */}, /* Gemm_134_gemm_11_1_reshape_x_40_inserted_out666 */
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
    LL_Streng_TensorInit(3, &Identity_inserted_id666_dma_init_out_0_134, 1);

    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_131 output ports=0 range=1[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_131_dma_init_out_0_134 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_131_out_0 */
      .offset_start = 204800,
      .offset_limit = 409664,
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
    LL_Streng_TensorInit(8, &Reshape_131_dma_init_out_0_134, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM6 <- 102400 */
    /* npuRAM5 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_134[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id667 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id666 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_131 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=134 */
    LL_Switch_Init(switch_init_in_134, 3);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_134_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_134_all_units, 6);

  }

  ec_trace_wait_epoch_end(0x148);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_134[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id667 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id666 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_131 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
    };


    /* epoch=134 */
    LL_Switch_Deinit(switch_deinit_in_134, 3);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_134_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_134_all_units, 6);

  }
  ec_trace_end_epoch(134);
  ec_trace_end_blob("_ec_blob_network_134");
}

void trace_ec__ec_blob_network_140(void) {
  ec_trace_start_blob("_ec_blob_network_140");
  ec_trace_start_epoch(140);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_134_out_0_0_10 */
    /* node=Gemm_134_out_0_0_10 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_134_out_0_0_14 */
    /* node=Gemm_134_out_0_0_14 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_10 input ports=0 range=11[0,640000] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_10_dma_init_in_0_140 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* ???_627 */
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
    LL_Streng_TensorInit(0, &Gemm_134_out_0_0_10_dma_init_in_0_140, 1);

    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_14 input ports=0 range=7[4915200,5555200] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_14_dma_init_in_0_140 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* ???_635 */
      .offset_start = 4915200,
      .offset_limit = 5555264,
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
    LL_Streng_TensorInit(7, &Gemm_134_out_0_0_14_dma_init_in_0_140, 1);


    /* Dma input bandwidth from memory pools: */
    /* cpuRAM2 -> 640000 */
    /* hyperRAM -> 640000 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_10 output ports=0 range=7[0,640000] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_10_dma_init_out_0_140 = {
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
    LL_Streng_TensorInit(6, &Gemm_134_out_0_0_10_dma_init_out_0_140, 1);

    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_134_out_0_0_14 output ports=0 range=7[640000,1280000] */

    static const LL_Streng_TensorInitTypeDef Gemm_134_out_0_0_14_dma_init_out_0_140 = {
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
    LL_Streng_TensorInit(3, &Gemm_134_out_0_0_14_dma_init_out_0_140, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 1280000 */

    static const LL_Switch_InitTypeDef switch_init_in_140[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_10 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_14 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=140 */
    LL_Switch_Init(switch_init_in_140, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_140_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_140_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x48);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_140[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_10 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_134_out_0_0_14 OUT: in unit=STREAM_ENG_V2 3 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=140 */
    LL_Switch_Deinit(switch_deinit_in_140, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_140_all_units[] = {
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_140_all_units, 4);

  }
  ec_trace_end_epoch(140);
  ec_trace_start_epoch(141);
  {
  }
  {
  }
  ec_trace_end_epoch(141);
  ec_trace_start_epoch(142);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id668 */
    /* node=Identity_inserted_id668 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id668 input ports=0 range=7[0,1280000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id668_dma_init_in_0_142 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_134_out_0_0_15_o_inserted_in668 */
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
    LL_Streng_TensorInit(8, &Identity_inserted_id668_dma_init_in_0_142, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1280000 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id668 output ports=0 range=7[1280000,2560000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id668_dma_init_out_0_142 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_134_out_0_0_15_o_inserted_out668 */
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
    LL_Streng_TensorInit(2, &Identity_inserted_id668_dma_init_out_0_142, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 1280000 */

    static const LL_Switch_InitTypeDef switch_init_in_142[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id668 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=142 */
    LL_Switch_Init(switch_init_in_142, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_142_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_142_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x4);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_142[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id668 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=142 */
    LL_Switch_Deinit(switch_deinit_in_142, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_142_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_142_all_units, 2);

  }
  ec_trace_end_epoch(142);
  ec_trace_end_blob("_ec_blob_network_140");
}

void trace_ec__ec_blob_network_146(void) {
  ec_trace_start_blob("_ec_blob_network_146");
  ec_trace_start_epoch(146);
  {
  }
  {
  }
  ec_trace_end_epoch(146);
  ec_trace_end_blob("_ec_blob_network_146");
}

void trace_ec__ec_blob_network_148(void) {
  ec_trace_start_blob("_ec_blob_network_148");
  ec_trace_start_epoch(148);
  {
  }
  {
  }
  ec_trace_end_epoch(148);
  ec_trace_start_epoch(149);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id670 */
    /* node=Identity_inserted_id670 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id669 */
    /* node=Identity_inserted_id669 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id670 input ports=0 range=11[1253376,1893376] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id670_dma_init_in_0_149 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Gemm_138_gemm_23_0_reshape_x_47_inserted_in670 */
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
    LL_Streng_TensorInit(3, &Identity_inserted_id670_dma_init_in_0_149, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id669 input ports=0 range=11[0,640000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id669_dma_init_in_0_149 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Gemm_138_gemm_27_1_reshape_x_54_inserted_in669 */
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
    LL_Streng_TensorInit(6, &Identity_inserted_id669_dma_init_in_0_149, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 386048 */
    /* npuRAM3 -> 253952 */
    /* cpuRAM2 -> 640000 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id670 output ports=0 range=7[4915200,5555200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id670_dma_init_out_0_149 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_138_gemm_23_0_reshape_x_47_inserted_out670 */
      .offset_start = 4915200,
      .offset_limit = 5555264,
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
    LL_Streng_TensorInit(2, &Identity_inserted_id670_dma_init_out_0_149, 1);

    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id669 output ports=0 range=7[5555200,6195200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id669_dma_init_out_0_149 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Gemm_138_gemm_27_1_reshape_x_54_inserted_out669 */
      .offset_start = 5555200,
      .offset_limit = 6195264,
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
    LL_Streng_TensorInit(7, &Identity_inserted_id669_dma_init_out_0_149, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 1280000 */

    static const LL_Switch_InitTypeDef switch_init_in_149[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id670 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id669 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=149 */
    LL_Switch_Init(switch_init_in_149, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_149_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_149_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x84);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_149[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id670 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id669 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=149 */
    LL_Switch_Deinit(switch_deinit_in_149, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_149_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_149_all_units, 4);

  }
  ec_trace_end_epoch(149);
  ec_trace_end_blob("_ec_blob_network_148");
}

void trace_ec__ec_blob_network_152(void) {
  ec_trace_start_blob("_ec_blob_network_152");
  ec_trace_start_epoch(152);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_138_out_0_16_26 */
    /* node=Gemm_138_out_0_16_26 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Gemm_138_out_0_16_30 */
    /* node=Gemm_138_out_0_16_30 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_26 input ports=0 range=2[0,102400] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_26_dma_init_in_0_152 = {
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
    LL_Streng_TensorInit(1, &Gemm_138_out_0_16_26_dma_init_in_0_152, 1);

    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_30 input ports=0 range=2[102400,204800] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_30_dma_init_in_0_152 = {
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
    LL_Streng_TensorInit(3, &Gemm_138_out_0_16_30_dma_init_in_0_152, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_26 output ports=0 range=1[204800,307200] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_26_dma_init_out_0_152 = {
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
    LL_Streng_TensorInit(6, &Gemm_138_out_0_16_26_dma_init_out_0_152, 1);

    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Gemm_138_out_0_16_30 output ports=0 range=1[307200,409600] */

    static const LL_Streng_TensorInitTypeDef Gemm_138_out_0_16_30_dma_init_out_0_152 = {
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
    LL_Streng_TensorInit(7, &Gemm_138_out_0_16_30_dma_init_out_0_152, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_152[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_26 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_30 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=152 */
    LL_Switch_Init(switch_init_in_152, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_152_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_152_all_units, 4);

  }

  ec_trace_wait_epoch_end(0xc0);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_152[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_26 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Gemm_138_out_0_16_30 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
    };


    /* epoch=152 */
    LL_Switch_Deinit(switch_deinit_in_152, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_152_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_152_all_units, 4);

  }
  ec_trace_end_epoch(152);
  ec_trace_start_epoch(153);
  {
  }
  {
  }
  ec_trace_end_epoch(153);
  ec_trace_start_epoch(154);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id671 */
    /* node=Identity_inserted_id671 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id671 input ports=0 range=1[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id671_dma_init_in_0_154 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_139_out_0_inserted_in671 */
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
    LL_Streng_TensorInit(5, &Identity_inserted_id671_dma_init_in_0_154, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM5 -> 204800 */

    /* Dma output units from cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id671 output ports=0 range=2[0,204800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id671_dma_init_out_0_154 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_139_out_0_inserted_out671 */
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
    LL_Streng_TensorInit(0, &Identity_inserted_id671_dma_init_out_0_154, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 204800 */

    static const LL_Switch_InitTypeDef switch_init_in_154[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id671 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=154 */
    LL_Switch_Init(switch_init_in_154, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_154_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_154_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x1);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_154[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id671 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
    };


    /* epoch=154 */
    LL_Switch_Deinit(switch_deinit_in_154, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_154_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_154_all_units, 2);

  }
  ec_trace_end_epoch(154);
  ec_trace_end_blob("_ec_blob_network_152");
}

void trace_ec__ec_blob_network_163(void) {
  ec_trace_start_blob("_ec_blob_network_163");
  ec_trace_start_epoch(163);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_148 */
    /* node=Concat_148 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_148 input ports=0 range=3[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_148_dma_init_in_0_163 = {
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
    LL_Streng_TensorInit(8, &Concat_148_dma_init_in_0_163, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM3 -> 409600 */

    /* Dma output units from cycle: */
    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_148 output ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_148_dma_init_out_0_163 = {
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
    LL_Streng_TensorInit(2, &Concat_148_dma_init_out_0_163, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 409600 */

    static const LL_Switch_InitTypeDef switch_init_in_163[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_148 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=163 */
    LL_Switch_Init(switch_init_in_163, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_163_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_163_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x4);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_163[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_148 OUT: in unit=STREAM_ENG_V2 2 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=163 */
    LL_Switch_Deinit(switch_deinit_in_163, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_163_all_units[] = {
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_163_all_units, 2);

  }
  ec_trace_end_epoch(163);
  ec_trace_end_blob("_ec_blob_network_163");
}

void trace_ec__ec_blob_network_167(void) {
  ec_trace_start_blob("_ec_blob_network_167");
  ec_trace_start_epoch(167);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Resize_152_resize_NN_expansion_concat_76 */
    /* node=Resize_152_resize_NN_expansion_concat_76 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_152_resize_NN_expansion_concat_76 input ports=0 range=0[0,409600] */

    static const LL_Streng_TensorInitTypeDef Resize_152_resize_NN_expansion_concat_76_dma_init_in_0_167 = {
      /* from memory with batch=256
iterating outer iter=0 num_higher_elem=4
spanning across 1638400 bytes */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34350000UL) /* Equivalent hex address = 0x34350000UL */}, /* Mul_151_out_0 */
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
      .frame_loop_cnt = 1,
      .frame_tot_cnt = 4,
      .nbits_in = 16,
      .nbits_out = 16,
    };

    /* Unit=STREAM_ENG_V2 */
    LL_Streng_TensorInit(6, &Resize_152_resize_NN_expansion_concat_76_dma_init_in_0_167, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM6 -> 1638400 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_152_resize_NN_expansion_concat_76 output ports=0 range=11[0,1638400] */

    static const LL_Streng_TensorInitTypeDef Resize_152_resize_NN_expansion_concat_76_dma_init_out_0_167 = {
      /* to memory canonical from batch=256 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Resize_152_resize_NN_expansion_concat_76_out_77 */
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
    LL_Streng_TensorInit(1, &Resize_152_resize_NN_expansion_concat_76_dma_init_out_0_167, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM4 <- 131072 */
    /* npuRAM3 <- 458752 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_167[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_152_resize_NN_expansion_concat_76 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=167 */
    LL_Switch_Init(switch_init_in_167, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_167_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_167_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_167[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_152_resize_NN_expansion_concat_76 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=167 */
    LL_Switch_Deinit(switch_deinit_in_167, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_167_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_167_all_units, 2);

  }
  ec_trace_end_epoch(167);
  ec_trace_end_blob("_ec_blob_network_167");
}

void trace_ec__ec_blob_network_181(void) {
  ec_trace_start_blob("_ec_blob_network_181");
  ec_trace_start_epoch(181);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_165 */
    /* node=Concat_165 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_165 input ports=0 range=7[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_165_dma_init_in_0_181 = {
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
    LL_Streng_TensorInit(7, &Concat_165_dma_init_in_0_181, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 1228800 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_165 output ports=0 range=11[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_165_dma_init_out_0_181 = {
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
    LL_Streng_TensorInit(1, &Concat_165_dma_init_out_0_181, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM3 <- 180224 */
    /* cpuRAM2 <- 1048576 */

    static const LL_Switch_InitTypeDef switch_init_in_181[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_165 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=181 */
    LL_Switch_Init(switch_init_in_181, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_181_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_181_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_181[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_165 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=181 */
    LL_Switch_Deinit(switch_deinit_in_181, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_181_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_181_all_units, 2);

  }
  ec_trace_end_epoch(181);
  ec_trace_end_blob("_ec_blob_network_181");
}

void trace_ec__ec_blob_network_185(void) {
  ec_trace_start_blob("_ec_blob_network_185");
  ec_trace_start_epoch(185);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Resize_169_resize_NN_expansion_concat_80 */
    /* node=Resize_169_resize_NN_expansion_concat_80 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_169_resize_NN_expansion_concat_80 input ports=0 range=7[13107200,13926400] */

    static const LL_Streng_TensorInitTypeDef Resize_169_resize_NN_expansion_concat_80_dma_init_in_0_185 = {
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
    LL_Streng_TensorInit(0, &Resize_169_resize_NN_expansion_concat_80_dma_init_in_0_185, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 3276800 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Resize_169_resize_NN_expansion_concat_80 output ports=0 range=7[0,3276800] */

    static const LL_Streng_TensorInitTypeDef Resize_169_resize_NN_expansion_concat_80_dma_init_out_0_185 = {
      /* to memory canonical from batch=128 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Resize_169_resize_NN_expansion_concat_80_out_81 */
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
    LL_Streng_TensorInit(9, &Resize_169_resize_NN_expansion_concat_80_dma_init_out_0_185, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 3276800 */

    static const LL_Switch_InitTypeDef switch_init_in_185[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_169_resize_NN_expansion_concat_80 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=185 */
    LL_Switch_Init(switch_init_in_185, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_185_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_185_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x200);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_185[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Resize_169_resize_NN_expansion_concat_80 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=185 */
    LL_Switch_Deinit(switch_deinit_in_185, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_185_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_185_all_units, 2);

  }
  ec_trace_end_epoch(185);
  ec_trace_end_blob("_ec_blob_network_185");
}

void trace_ec__ec_blob_network_187(void) {
  ec_trace_start_blob("_ec_blob_network_187");
  ec_trace_start_epoch(187);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_170 */
    /* node=Concat_170 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_170 input ports=0 range=7[6553600,13107200] */

    static const LL_Streng_TensorInitTypeDef Concat_170_dma_init_in_0_187 = {
      /* from memory with batch=128 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Resize_169_resize_NN_expansion_concat_80_out_83 */
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
    LL_Streng_TensorInit(8, &Concat_170_dma_init_in_0_187, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 6553600 */

    /* Dma output units from cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_170 output ports=0 range=7[0,6553600] */

    static const LL_Streng_TensorInitTypeDef Concat_170_dma_init_out_0_187 = {
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
    LL_Streng_TensorInit(0, &Concat_170_dma_init_out_0_187, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 6553600 */

    static const LL_Switch_InitTypeDef switch_init_in_187[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_170 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=187 */
    LL_Switch_Init(switch_init_in_187, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_187_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_187_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x1);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_187[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_170 OUT: in unit=STREAM_ENG_V2 0 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
    };


    /* epoch=187 */
    LL_Switch_Deinit(switch_deinit_in_187, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_187_all_units[] = {
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_187_all_units, 2);

  }
  ec_trace_end_epoch(187);
  ec_trace_end_blob("_ec_blob_network_187");
}

void trace_ec__ec_blob_network_199(void) {
  ec_trace_start_blob("_ec_blob_network_199");
  ec_trace_start_epoch(199);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_182 */
    /* node=Concat_182 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_182 input ports=0 range=7[0,2457600] */

    static const LL_Streng_TensorInitTypeDef Concat_182_dma_init_in_0_199 = {
      /* from memory with batch=32 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
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
    LL_Streng_TensorInit(0, &Concat_182_dma_init_in_0_199, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2457600 */

    /* Dma output units from cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_182 output ports=0 range=7[2457600,4915200] */

    static const LL_Streng_TensorInitTypeDef Concat_182_dma_init_out_0_199 = {
      /* to memory canonical from batch=32 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Concat_182_out_0 */
      .offset_start = 2457600,
      .offset_limit = 4915264,
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
    LL_Streng_TensorInit(7, &Concat_182_dma_init_out_0_199, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 2457600 */

    static const LL_Switch_InitTypeDef switch_init_in_199[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_182 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=199 */
    LL_Switch_Init(switch_init_in_199, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_199_all_units[] = {
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_199_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x80);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_199[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_182 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
    };


    /* epoch=199 */
    LL_Switch_Deinit(switch_deinit_in_199, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_199_all_units[] = {
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_199_all_units, 2);

  }
  ec_trace_end_epoch(199);
  ec_trace_end_blob("_ec_blob_network_199");
}

void trace_ec__ec_blob_network_245(void) {
  ec_trace_start_blob("_ec_blob_network_245");
  ec_trace_start_epoch(245);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_231 */
    /* node=Concat_231 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_193 */
    /* node=Reshape_193 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_215 */
    /* node=Reshape_215 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 3 [STREAM_ENG_V2 3] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_231 input ports=0 range=7[3788800,5017600] */

    static const LL_Streng_TensorInitTypeDef Concat_231_dma_init_in_0_245 = {
      /* from memory with batch=64 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Split_223_out_0 */
      .offset_start = 3788800,
      .offset_end = 4198400,
      .offset_limit = 5017664,
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
    LL_Streng_TensorInit(3, &Concat_231_dma_init_in_0_245, 1);

    /* Unit= 2 [STREAM_ENG_V2 2] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_193 input ports=0 range=7[13926400,14387200] */

    static const LL_Streng_TensorInitTypeDef Reshape_193_dma_init_in_0_245 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Conv2D_192_out_0 */
      .offset_start = 13926400,
      .offset_limit = 14387264,
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
    LL_Streng_TensorInit(2, &Reshape_193_dma_init_in_0_245, 1);

    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_215 input ports=0 range=7[0,1638400] */

    static const LL_Streng_TensorInitTypeDef Reshape_215_dma_init_in_0_245 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Conv2D_214_out_0 */
      .offset_start = 0,
      .offset_limit = 1638464,
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
    LL_Streng_TensorInit(4, &Reshape_215_dma_init_in_0_245, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 3328000 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_231 output ports=0 range=11[0,1228800] */

    static const LL_Streng_TensorInitTypeDef Concat_231_dma_init_out_0_245 = {
      /* to memory canonical from batch=64 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Concat_231_out_0 */
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
    LL_Streng_TensorInit(9, &Concat_231_dma_init_out_0_245, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_193 output ports=0 range=7[1638400,2099200] */

    static const LL_Streng_TensorInitTypeDef Reshape_193_dma_init_out_0_245 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Reshape_193_out_0 */
      .offset_start = 1638400,
      .offset_end = 2099200,
      .offset_limit = 2099264,
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
    LL_Streng_TensorInit(1, &Reshape_193_dma_init_out_0_245, 1);

    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_215 output ports=0 range=7[2150400,3788800] */

    static const LL_Streng_TensorInitTypeDef Reshape_215_dma_init_out_0_245 = {
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
    LL_Streng_TensorInit(6, &Reshape_215_dma_init_out_0_245, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM3 <- 180224 */
    /* cpuRAM2 <- 1048576 */
    /* hyperRAM <- 2099200 */

    static const LL_Switch_InitTypeDef switch_init_in_245[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_231 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_193 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_215 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=245 */
    LL_Switch_Init(switch_init_in_245, 3);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_245_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_245_all_units, 6);

  }

  ec_trace_wait_epoch_end(0x242);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_245[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 3, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_231 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 3 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 2, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_193 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 2 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_215 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=245 */
    LL_Switch_Deinit(switch_deinit_in_245, 3);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_245_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 2} }, /* STREAM_ENG_V2 */
      { {STRENG, 3} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_245_all_units, 6);

  }
  ec_trace_end_epoch(245);
  ec_trace_end_blob("_ec_blob_network_245");
}

void trace_ec__ec_blob_network_304(void) {
  ec_trace_start_blob("_ec_blob_network_304");
  ec_trace_start_epoch(304);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_293 */
    /* node=Concat_293 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_242 */
    /* node=Reshape_242 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_264 */
    /* node=Reshape_264 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 5 [STREAM_ENG_V2 5] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_293 input ports=0 range=2[0,204800] */

    static const LL_Streng_TensorInitTypeDef Concat_293_dma_init_in_0_304 = {
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
    LL_Streng_TensorInit(5, &Concat_293_dma_init_in_0_304, 1);

    /* Unit= 0 [STREAM_ENG_V2 0] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_242 input ports=0 range=11[819200,934400] */

    static const LL_Streng_TensorInitTypeDef Reshape_242_dma_init_in_0_304 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Conv2D_241_out_0 */
      .offset_start = 819200,
      .offset_limit = 934464,
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
    LL_Streng_TensorInit(0, &Reshape_242_dma_init_in_0_304, 1);

    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_264 input ports=0 range=11[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_264_dma_init_in_0_304 = {
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
    LL_Streng_TensorInit(9, &Reshape_264_dma_init_in_0_304, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 204800 */
    /* cpuRAM2 -> 524800 */

    /* Dma output units from cycle: */
    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_293 output ports=0 range=2[204800,409600] */

    static const LL_Streng_TensorInitTypeDef Concat_293_dma_init_out_0_304 = {
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
    LL_Streng_TensorInit(7, &Concat_293_dma_init_out_0_304, 1);

    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_242 output ports=0 range=11[614400,729600] */

    static const LL_Streng_TensorInitTypeDef Reshape_242_dma_init_out_0_304 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Reshape_242_out_0 */
      .offset_start = 614400,
      .offset_end = 729600,
      .offset_limit = 729664,
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
    LL_Streng_TensorInit(4, &Reshape_242_dma_init_out_0_304, 1);

    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_264 output ports=0 range=1[0,409600] */

    static const LL_Streng_TensorInitTypeDef Reshape_264_dma_init_out_0_304 = {
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
    LL_Streng_TensorInit(1, &Reshape_264_dma_init_out_0_304, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 409600 */
    /* npuRAM4 <- 204800 */
    /* cpuRAM2 <- 115200 */

    static const LL_Switch_InitTypeDef switch_init_in_304[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_293 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_242 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_264 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=304 */
    LL_Switch_Init(switch_init_in_304, 3);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_304_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_304_all_units, 6);

  }

  ec_trace_wait_epoch_end(0x92);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_304[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 5, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_293 OUT: in unit=STREAM_ENG_V2 7 in port=0 out unit=STREAM_ENG_V2 5 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_242 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 0 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_264 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 9 out port=0 */
    };


    /* epoch=304 */
    LL_Switch_Deinit(switch_deinit_in_304, 3);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_304_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 0} }, /* STREAM_ENG_V2 */
      { {STRENG, 5} }, /* STREAM_ENG_V2 */
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_304_all_units, 6);

  }
  ec_trace_end_epoch(304);
  ec_trace_end_blob("_ec_blob_network_304");
}

void trace_ec__ec_blob_network_308(void) {
  ec_trace_start_blob("_ec_blob_network_308");
  ec_trace_start_epoch(308);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Concat node=Concat_297 */
    /* node=Concat_297 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_297 input ports=0 range=7[0,614400] */

    static const LL_Streng_TensorInitTypeDef Concat_297_dma_init_in_0_308 = {
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
    LL_Streng_TensorInit(1, &Concat_297_dma_init_in_0_308, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 614400 */

    /* Dma output units from cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Concat_297 output ports=0 range=11[0,614400] */

    static const LL_Streng_TensorInitTypeDef Concat_297_dma_init_out_0_308 = {
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
    LL_Streng_TensorInit(8, &Concat_297_dma_init_out_0_308, 1);


    /* Dma output bandwidth to memory pools: */
    /* cpuRAM2 <- 614400 */

    static const LL_Switch_InitTypeDef switch_init_in_308[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_297 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=308 */
    LL_Switch_Init(switch_init_in_308, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_308_all_units[] = {
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_308_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x100);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_308[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Concat_297 OUT: in unit=STREAM_ENG_V2 8 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=308 */
    LL_Switch_Deinit(switch_deinit_in_308, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_308_all_units[] = {
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_308_all_units, 2);

  }
  ec_trace_end_epoch(308);
  ec_trace_end_blob("_ec_blob_network_308");
}

void trace_ec__ec_blob_network_339(void) {
  ec_trace_start_blob("_ec_blob_network_339");
  ec_trace_start_epoch(339);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_308 */
    /* node=Reshape_308 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_342 */
    /* node=Reshape_342 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 8 [STREAM_ENG_V2 8] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_308 input ports=0 range=2[409600,438400] */

    static const LL_Streng_TensorInitTypeDef Reshape_308_dma_init_in_0_339 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Conv2D_307_out_0 */
      .offset_start = 409600,
      .offset_limit = 438464,
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
    LL_Streng_TensorInit(8, &Reshape_308_dma_init_in_0_339, 1);

    /* Unit= 7 [STREAM_ENG_V2 7] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_342 input ports=0 range=2[204800,307200] */

    static const LL_Streng_TensorInitTypeDef Reshape_342_dma_init_in_0_339 = {
      /* memory canonical to batch=1 */
      .dir = 0,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Conv2D_341_out_0 */
      .offset_start = 204800,
      .offset_limit = 307264,
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
    LL_Streng_TensorInit(7, &Reshape_342_dma_init_in_0_339, 1);


    /* Dma input bandwidth from memory pools: */
    /* npuRAM4 -> 131200 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_308 output ports=0 range=1[409600,438400] */

    static const LL_Streng_TensorInitTypeDef Reshape_308_dma_init_out_0_339 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x342e0000UL) /* Equivalent hex address = 0x342e0000UL */}, /* Reshape_308_out_0 */
      .offset_start = 409600,
      .offset_end = 438400,
      .offset_limit = 438464,
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
    LL_Streng_TensorInit(6, &Reshape_308_dma_init_out_0_339, 1);

    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_342 output ports=0 range=2[0,102400] */

    static const LL_Streng_TensorInitTypeDef Reshape_342_dma_init_out_0_339 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34270000UL) /* Equivalent hex address = 0x34270000UL */}, /* Reshape_342_out_0 */
      .offset_start = 0,
      .offset_end = 102400,
      .offset_limit = 102464,
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
    LL_Streng_TensorInit(4, &Reshape_342_dma_init_out_0_339, 1);


    /* Dma output bandwidth to memory pools: */
    /* npuRAM5 <- 28800 */
    /* npuRAM4 <- 102400 */

    static const LL_Switch_InitTypeDef switch_init_in_339[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_308 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_342 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=339 */
    LL_Switch_Init(switch_init_in_339, 2);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_339_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_339_all_units, 4);

  }

  ec_trace_wait_epoch_end(0x50);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_339[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 8, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_308 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 8 out port=0 */
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 7, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_342 OUT: in unit=STREAM_ENG_V2 4 in port=0 out unit=STREAM_ENG_V2 7 out port=0 */
    };


    /* epoch=339 */
    LL_Switch_Deinit(switch_deinit_in_339, 2);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_339_all_units[] = {
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 7} }, /* STREAM_ENG_V2 */
      { {STRENG, 8} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_339_all_units, 4);

  }
  ec_trace_end_epoch(339);
  ec_trace_end_blob("_ec_blob_network_339");
}

void trace_ec__ec_blob_network_344(void) {
  ec_trace_start_blob("_ec_blob_network_344");
  ec_trace_start_epoch(344);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id673 */
    /* node=Identity_inserted_id673 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id673 input ports=0 range=11[0,604800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id673_dma_init_in_0_344 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x34100000UL) /* Equivalent hex address = 0x34100000UL */}, /* Reshape_310_out_0_inserted_in673 */
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
    LL_Streng_TensorInit(6, &Identity_inserted_id673_dma_init_in_0_344, 1);


    /* Dma input bandwidth from memory pools: */
    /* cpuRAM2 -> 604800 */

    /* Dma output units from cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id673 output ports=0 range=7[6451200,7056000] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id673_dma_init_out_0_344 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Reshape_310_out_0_inserted_out673 */
      .offset_start = 6451200,
      .offset_limit = 7056064,
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
    LL_Streng_TensorInit(1, &Identity_inserted_id673_dma_init_out_0_344, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 604800 */

    static const LL_Switch_InitTypeDef switch_init_in_344[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id673 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=344 */
    LL_Switch_Init(switch_init_in_344, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_344_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_344_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x2);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_344[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id673 OUT: in unit=STREAM_ENG_V2 1 in port=0 out unit=STREAM_ENG_V2 6 out port=0 */
    };


    /* epoch=344 */
    LL_Switch_Deinit(switch_deinit_in_344, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_344_all_units[] = {
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_344_all_units, 2);

  }
  ec_trace_end_epoch(344);
  ec_trace_end_blob("_ec_blob_network_344");
}

void trace_ec__ec_blob_network_346(void) {
  ec_trace_start_blob("_ec_blob_network_346");
  ec_trace_start_epoch(346);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Identity node=Identity_inserted_id674 */
    /* node=Identity_inserted_id674 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 1 [STREAM_ENG_V2 1] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id674 input ports=0 range=7[2150400,4300800] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id674_dma_init_in_0_346 = {
      /* from memory with batch=1 */
      .dir = 0,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Transpose_345_out_0_inserted_in674 */
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
    LL_Streng_TensorInit(1, &Identity_inserted_id674_dma_init_in_0_346, 1);


    /* Dma input bandwidth from memory pools: */
    /* hyperRAM -> 2150400 */

    /* Dma output units from cycle: */
    /* Unit= 9 [STREAM_ENG_V2 9] */
    /* Emit conf for STREAM_ENG_V2 node=Identity_inserted_id674 output ports=0 range=7[4300800,6451200] */

    static const LL_Streng_TensorInitTypeDef Identity_inserted_id674_dma_init_out_0_346 = {
      /* to memory canonical from batch=1 */
      .dir = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Transpose_345_out_0_inserted_out674 */
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
    LL_Streng_TensorInit(9, &Identity_inserted_id674_dma_init_out_0_346, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 2150400 */

    static const LL_Switch_InitTypeDef switch_init_in_346[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id674 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=346 */
    LL_Switch_Init(switch_init_in_346, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_346_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_346_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x200);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_346[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 9, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 1, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Identity_inserted_id674 OUT: in unit=STREAM_ENG_V2 9 in port=0 out unit=STREAM_ENG_V2 1 out port=0 */
    };


    /* epoch=346 */
    LL_Switch_Deinit(switch_deinit_in_346, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_346_all_units[] = {
      { {STRENG, 9} }, /* STREAM_ENG_V2 */
      { {STRENG, 1} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_346_all_units, 2);

  }
  ec_trace_end_epoch(346);
  ec_trace_end_blob("_ec_blob_network_346");
}

void trace_ec__ec_blob_network_354(void) {
  ec_trace_start_blob("_ec_blob_network_354");
  ec_trace_start_epoch(354);
  {
    /* Unit= 28 [NULL_UNIT 0] */
    /* kind=Reshape node=Reshape_318 */
    /* node=Reshape_318 satisfies input and output adjacency (DMA->DMA) and can be omitted */

    /* Dma inputs units to cycle: */
    /* Unit= 4 [STREAM_ENG_V2 4] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_318 input ports=0 range=11[0,604800] */

    static const LL_Streng_TensorInitTypeDef Reshape_318_dma_init_in_0_354 = {
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
    LL_Streng_TensorInit(4, &Reshape_318_dma_init_in_0_354, 1);


    /* Dma input bandwidth from memory pools: */
    /* cpuRAM2 -> 604800 */

    /* Dma output units from cycle: */
    /* Unit= 6 [STREAM_ENG_V2 6] */
    /* Emit conf for STREAM_ENG_V2 node=Reshape_318 output ports=0 range=7[0,604800] */

    static const LL_Streng_TensorInitTypeDef Reshape_318_dma_init_out_0_354 = {
      /* to memory with batch=1 */
      .dir = 1,
      .raw = 1,
      .noblk = 0,
      .align_right = 0,
      .nbits_unsigned = 0,
      .cacheable = 1,
      .cache_allocate = 0,
      .addr_base = {(unsigned char *)(0x90000000UL) /* Equivalent hex address = 0x90000000UL */}, /* Reshape_318_out_0 */
      .offset_start = 0,
      .offset_end = 604800,
      .offset_limit = 604864,
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
    LL_Streng_TensorInit(6, &Reshape_318_dma_init_out_0_354, 1);


    /* Dma output bandwidth to memory pools: */
    /* hyperRAM <- 604800 */

    static const LL_Switch_InitTypeDef switch_init_in_354[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_318 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=354 */
    LL_Switch_Init(switch_init_in_354, 1);

    static const LL_ATON_EnableUnits_InitTypeDef Enable_epoch_354_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_EnableUnits_Init(Enable_epoch_354_all_units, 2);

  }

  ec_trace_wait_epoch_end(0x40);

  {
    static const LL_Switch_DeinitTypeDef switch_deinit_in_354[] = {
      { LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 6, 0), LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 4, 0), LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0, }, /* Reshape_318 OUT: in unit=STREAM_ENG_V2 6 in port=0 out unit=STREAM_ENG_V2 4 out port=0 */
    };


    /* epoch=354 */
    LL_Switch_Deinit(switch_deinit_in_354, 1);

    static const LL_ATON_DisableUnits_InitTypeDef Disable_epoch_354_all_units[] = {
      { {STRENG, 6} }, /* STREAM_ENG_V2 */
      { {STRENG, 4} }, /* STREAM_ENG_V2 */
    };


    LL_ATON_DisableUnits_Init(Disable_epoch_354_all_units, 2);

  }
  ec_trace_end_epoch(354);
  ec_trace_end_blob("_ec_blob_network_354");
}

void trace_ec__ec_blob_network_360(void) {
  ec_trace_start_blob("_ec_blob_network_360");
  ec_trace_start_epoch(360);
  {
  }
  {
  }
  ec_trace_end_epoch(360);
  ec_trace_end_blob("_ec_blob_network_360");
}


int main () {
  ec_trace_init("network_ecblobs.h", "network", false);
  trace_ec__ec_blob_network_1();
  trace_ec__ec_blob_network_19();
  trace_ec__ec_blob_network_37();
  trace_ec__ec_blob_network_68();
  trace_ec__ec_blob_network_72();
  trace_ec__ec_blob_network_103();
  trace_ec__ec_blob_network_107();
  trace_ec__ec_blob_network_121();
  trace_ec__ec_blob_network_130();
  trace_ec__ec_blob_network_134();
  trace_ec__ec_blob_network_140();
  trace_ec__ec_blob_network_146();
  trace_ec__ec_blob_network_148();
  trace_ec__ec_blob_network_152();
  trace_ec__ec_blob_network_163();
  trace_ec__ec_blob_network_167();
  trace_ec__ec_blob_network_181();
  trace_ec__ec_blob_network_185();
  trace_ec__ec_blob_network_187();
  trace_ec__ec_blob_network_199();
  trace_ec__ec_blob_network_245();
  trace_ec__ec_blob_network_304();
  trace_ec__ec_blob_network_308();
  trace_ec__ec_blob_network_339();
  trace_ec__ec_blob_network_344();
  trace_ec__ec_blob_network_346();
  trace_ec__ec_blob_network_354();
  trace_ec__ec_blob_network_360();
  ec_trace_all_blobs_done();
}
