/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright(C) 2015 Linaro Limited. All rights reserved.
 * Author: Mathieu Poirier <mathieu.poirier@linaro.org>
 */

#ifndef _CORESIGHT_ETM_PERF_H
#define _CORESIGHT_ETM_PERF_H

#include <linux/percpu-defs.h>
#include "coresight-priv.h"

struct coresight_device;
struct cscfg_config_desc;

/*
 * In both ETMv3 and v4 the maximum number of address comparator implentable
 * is 8.  The actual number is implementation specific and will be checked
 * when filters are applied.
 */
#define ETM_ADDR_CMP_MAX	8

/**
 * struct etm_filter - single instruction range or start/stop configuration.
 * @start_addr:	The address to start tracing on.
 * @stop_addr:	The address to stop tracing on.
 * @type:	Is this a range or start/stop filter.
 */
struct etm_filter {
	unsigned long start_addr;
	unsigned long stop_addr;
	enum etm_addr_type type;
};

/**
 * struct etm_filters - set of filters for a session
 * @etm_filter:	All the filters for this session.
 * @nr_filters:	Number of filters
 * @ssstatus:	Status of the start/stop logic.
 */
struct etm_filters {
	struct etm_filter	etm_filter[ETM_ADDR_CMP_MAX];
	unsigned int		nr_filters;
	bool			ssstatus;
};

/**
 * struct etm_event_data - Coresight specifics associated to an event
 * @work:		Handle to free allocated memory outside IRQ context.
 * @mask:		Hold the CPU(s) this event was set for.
 * @aux_hwid_done:	Whether a CPU has emitted the TraceID packet or not.
 * @snk_config:		The sink configuration.
 * @cfg_hash:		The hash id of any coresight config selected.
 * @path:		An array of path, each slot for one CPU.
 */
struct etm_event_data {
	struct work_struct work;
	cpumask_t mask;
	cpumask_t aux_hwid_done;
	void *snk_config;
	u32 cfg_hash;
	struct coresight_path * __percpu *path;
};

int etm_perf_symlink(struct coresight_device *csdev, bool link);
int etm_perf_add_symlink_sink(struct coresight_device *csdev);
void etm_perf_del_symlink_sink(struct coresight_device *csdev);
static inline void *etm_perf_sink_config(struct perf_output_handle *handle)
{
	struct etm_event_data *data = perf_get_aux(handle);

	if (data)
		return data->snk_config;
	return NULL;
}
int etm_perf_add_symlink_cscfg(struct device *dev,
			       struct cscfg_config_desc *config_desc);
void etm_perf_del_symlink_cscfg(struct cscfg_config_desc *config_desc);
int __init etm_perf_init(void);
void etm_perf_exit(void);

#endif
