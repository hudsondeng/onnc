#pragma once

#include <stdint.h>
#include <stdbool.h>
#ifndef MKLDNN
#define MKLDNN
#include "mkldnn.h"
#include <stdio.h>
typedef struct {
  int nargs;
  mkldnn_exec_arg_t *args;
} args_t;

static void prepare_arg_node(args_t *node, int nargs) {
  node->args = (mkldnn_exec_arg_t *)malloc(sizeof(mkldnn_exec_arg_t) * nargs);
  node->nargs = nargs;
}
static void set_arg(
    mkldnn_exec_arg_t *arg, int arg_idx, mkldnn_memory_t memory) {
  arg->arg = arg_idx;
  arg->memory = memory;
}
#endif

void ONNC_RUNTIME_gemm_float(
  void * restrict onnc_runtime_context
  ,const float * restrict input_A
  ,int32_t input_A_ndim, const int32_t * restrict input_A_dims
  ,const float * restrict input_B
  ,int32_t input_B_ndim, const int32_t * restrict input_B_dims
  ,const float * restrict input_C
  ,int32_t input_C_ndim, const int32_t * restrict input_C_dims
  ,float * restrict output_Y
  ,int32_t output_Y_ndim, const int32_t * restrict output_Y_dims
  ,float alpha
  ,float beta
  ,int32_t transA
  ,int32_t transB
);
