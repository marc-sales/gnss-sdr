/*!
 * \file gps_l1_ca_dll_pll_optim_tracking_cc.h
 * \brief Implementation of a code DLL + carrier PLL tracking block
 * \author Javier Arribas, 2012. jarribas(at)cttc.es
 * GPS L1 TRACKING MODULE OPTIMIZED FOR SPEED:
 * - Code Doppler is not compensated in the local replica
 * Code DLL + carrier PLL according to the algorithms described in:
 * [1] K.Borre, D.M.Akos, N.Bertelsen, P.Rinder, and S.H.Jensen,
 * A Software-Defined GPS and Galileo Receiver. A Single-Frequency
 * Approach, Birkha user, 2007
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#ifndef GNSS_SDR_GPS_L1_CA_DLL_PLL_OPTIM_TRACKING_CC_H
#define GNSS_SDR_GPS_L1_CA_DLL_PLL_OPTIM_TRACKING_CC_H

#include <fstream>
#include <queue>
#include <map>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <gnuradio/block.h>
#include <gnuradio/msg_queue.h>
#include "concurrent_queue.h"
#include "gps_sdr_signal_processing.h"
#include "gnss_synchro.h"
#include "tracking_2nd_DLL_filter.h"
#include "tracking_2nd_PLL_filter.h"
#include "correlator.h"

class Gps_L1_Ca_Dll_Pll_Optim_Tracking_cc;

typedef boost::shared_ptr<Gps_L1_Ca_Dll_Pll_Optim_Tracking_cc> gps_l1_ca_dll_pll_optim_tracking_cc_sptr;

gps_l1_ca_dll_pll_optim_tracking_cc_sptr gps_l1_ca_dll_pll_make_optim_tracking_cc(long if_freq,
                                                                                  long fs_in,
                                                                                  unsigned int vector_length,
                                                                                  boost::shared_ptr<gr::msg_queue> queue,
                                                                                  bool dump,
                                                                                  std::string dump_filename,
                                                                                  float pll_bw_hz,
                                                                                  float dll_bw_hz,
                                                                                  float early_late_space_chips);


/*!
 * \brief This class implements a DLL + PLL tracking loop block
 */
class Gps_L1_Ca_Dll_Pll_Optim_Tracking_cc: public gr::block
{
public:
    ~Gps_L1_Ca_Dll_Pll_Optim_Tracking_cc();

    void set_channel(unsigned int channel);
    void set_gnss_synchro(Gnss_Synchro* p_gnss_synchro);
    void start_tracking();
    void set_channel_queue(concurrent_queue<int> *channel_internal_queue);

    int general_work (int noutput_items, gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items, gr_vector_void_star &output_items);

    void forecast (int noutput_items, gr_vector_int &ninput_items_required);
private:
    friend gps_l1_ca_dll_pll_optim_tracking_cc_sptr
    gps_l1_ca_dll_pll_make_optim_tracking_cc(long if_freq,
            long fs_in, unsigned
            int vector_length,
            boost::shared_ptr<gr::msg_queue> queue,
            bool dump,
            std::string dump_filename,
            float pll_bw_hz,
            float dll_bw_hz,
            float early_late_space_chips);

    Gps_L1_Ca_Dll_Pll_Optim_Tracking_cc(long if_freq,
            long fs_in, unsigned
            int vector_length,
            boost::shared_ptr<gr::msg_queue> queue,
            bool dump,
            std::string dump_filename,
            float pll_bw_hz,
            float dll_bw_hz,
            float early_late_space_chips);
    void update_local_code();
    void update_local_carrier();

    // tracking configuration vars
    boost::shared_ptr<gr::msg_queue> d_queue;
    concurrent_queue<int> *d_channel_internal_queue;
    unsigned int d_vector_length;
    bool d_dump;

    Gnss_Synchro* d_acquisition_gnss_synchro;
    unsigned int d_channel;
    int d_last_seg;
    long d_if_freq;
    long d_fs_in;

    double d_early_late_spc_chips;

    gr_complex* d_ca_code;

    gr_complex* d_early_code;
    gr_complex* d_late_code;
    gr_complex* d_prompt_code;
    gr_complex* d_carr_sign;

    gr_complex *d_Early;
    gr_complex *d_Prompt;
    gr_complex *d_Late;

    // remaining code phase and carrier phase between tracking loops
    float d_rem_code_phase_samples;
    float d_rem_carr_phase_rad;

    // PLL and DLL filter library
    Tracking_2nd_DLL_filter d_code_loop_filter;
    Tracking_2nd_PLL_filter d_carrier_loop_filter;

    // acquisition
    float d_acq_code_phase_samples;
    float d_acq_carrier_doppler_hz;
    // correlator
    Correlator d_correlator;

    // tracking vars
    float d_code_freq_chips;
    float d_carrier_doppler_hz;
    float d_acc_carrier_phase_rad;
    float d_code_phase_samples;
    float d_acc_code_phase_secs;

    //PRN period in samples
    int d_current_prn_length_samples;

    //processing samples counters
    unsigned long int d_sample_counter;
    unsigned long int d_acq_sample_stamp;

    // CN0 estimation and lock detector
    int d_cn0_estimation_counter;
    gr_complex* d_Prompt_buffer;
    float d_carrier_lock_test;
    float d_CN0_SNV_dB_Hz;
    float d_carrier_lock_threshold;
    int d_carrier_lock_fail_counter;

    // control vars
    int d_gnuradio_forecast_samples;
    bool d_enable_tracking;
    bool d_pull_in;

    // file dump
    std::string d_dump_filename;
    std::ofstream d_dump_file;

    std::map<std::string, std::string> systemName;
    std::string sys;
};

#endif //GNSS_SDR_GPS_L1_CA_DLL_PLL_OPTIM_TRACKING_CC_H
