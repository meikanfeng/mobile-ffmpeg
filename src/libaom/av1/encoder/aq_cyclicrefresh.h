/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef AOM_AV1_ENCODER_AQ_CYCLICREFRESH_H_
#define AOM_AV1_ENCODER_AQ_CYCLICREFRESH_H_

#include "av1/common/blockd.h"

#ifdef __cplusplus
extern "C" {
#endif

// The segment ids used in cyclic refresh: from base (no boost) to increasing
// boost (higher delta-qp).
#define CR_SEGMENT_ID_BASE 0
#define CR_SEGMENT_ID_BOOST1 1
#define CR_SEGMENT_ID_BOOST2 2

// Maximum rate target ratio for setting segment delta-qp.
#define CR_MAX_RATE_TARGET_RATIO 4.0

struct CYCLIC_REFRESH {
  // Percentage of blocks per frame that are targeted as candidates
  // for cyclic refresh.
  int percent_refresh;
  // Maximum q-delta as percentage of base q.
  int max_qdelta_perc;
  // Superblock starting index for cycling through the frame.
  int sb_index;
  // Controls how long block will need to wait to be refreshed again, in
  // excess of the cycle time, i.e., in the case of all zero motion, block
  // will be refreshed every (100/percent_refresh + time_for_refresh) frames.
  int time_for_refresh;
  // Target number of (4x4) blocks that are set for delta-q.
  int target_num_seg_blocks;
  // Actual number of (4x4) blocks that were applied delta-q.
  int actual_num_seg1_blocks;
  int actual_num_seg2_blocks;
  // RD mult. parameters for segment 1.
  int rdmult;
  // Cyclic refresh map.
  int8_t *map;
  // Map of the last q a block was coded at.
  uint8_t *last_coded_q_map;
  // Thresholds applied to the projected rate/distortion of the coding block,
  // when deciding whether block should be refreshed.
  int64_t thresh_rate_sb;
  int64_t thresh_dist_sb;
  // Threshold applied to the motion vector (in units of 1/8 pel) of the
  // coding block, when deciding whether block should be refreshed.
  int16_t motion_thresh;
  // Rate target ratio to set q delta.
  double rate_ratio_qdelta;
  // Boost factor for rate target ratio, for segment CR_SEGMENT_ID_BOOST2.
  int rate_boost_fac;
  double low_content_avg;
  int qindex_delta[3];
  double weight_segment;
  int apply_cyclic_refresh;
};

struct AV1_COMP;

typedef struct CYCLIC_REFRESH CYCLIC_REFRESH;

CYCLIC_REFRESH *av1_cyclic_refresh_alloc(int mi_rows, int mi_cols);

void av1_cyclic_refresh_free(CYCLIC_REFRESH *cr);

// Estimate the bits, incorporating the delta-q from segment 1, after encoding
// the frame.
int av1_cyclic_refresh_estimate_bits_at_q(const struct AV1_COMP *cpi,
                                          double correction_factor);

// Estimate the bits per mb, for a given q = i and a corresponding delta-q
// (for segment 1), prior to encoding the frame.
int av1_cyclic_refresh_rc_bits_per_mb(const struct AV1_COMP *cpi, int i,
                                      double correction_factor);

// Prior to coding a given prediction block, of size bsize at (mi_row, mi_col),
// check if we should reset the segment_id, and update the cyclic_refresh map
// and segmentation map.
void av1_cyclic_refresh_update_segment(const struct AV1_COMP *cpi,
                                       MB_MODE_INFO *const mbmi, int mi_row,
                                       int mi_col, BLOCK_SIZE bsize,
                                       int64_t rate, int64_t dist, int skip);

// Update the actual number of blocks that were applied the segment delta q.
void av1_cyclic_refresh_postencode(struct AV1_COMP *const cpi);

// Set golden frame update interval, for 1 pass CBR mode.
void av1_cyclic_refresh_set_golden_update(struct AV1_COMP *const cpi);

// Set/update global/frame level refresh parameters.
void av1_cyclic_refresh_update_parameters(struct AV1_COMP *const cpi);

// Setup cyclic background refresh: set delta q and segmentation map.
void av1_cyclic_refresh_setup(struct AV1_COMP *const cpi);

int av1_cyclic_refresh_get_rdmult(const CYCLIC_REFRESH *cr);

void av1_cyclic_refresh_reset_resize(struct AV1_COMP *const cpi);

static INLINE int cyclic_refresh_segment_id_boosted(int segment_id) {
  return segment_id == CR_SEGMENT_ID_BOOST1 ||
         segment_id == CR_SEGMENT_ID_BOOST2;
}

static INLINE int cyclic_refresh_segment_id(int segment_id) {
  if (segment_id == CR_SEGMENT_ID_BOOST1)
    return CR_SEGMENT_ID_BOOST1;
  else if (segment_id == CR_SEGMENT_ID_BOOST2)
    return CR_SEGMENT_ID_BOOST2;
  else
    return CR_SEGMENT_ID_BASE;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // AOM_AV1_ENCODER_AQ_CYCLICREFRESH_H_
