/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/types.h>
#include <linux/videodev2.h>

#include <sys/ioctl.h>

struct strval32 {
	uint32_t val;
	const char *str;
};

int
main(int argc, char **argv)
{
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (unsigned int i = 0; i < num_skip; i++) {
		long rc = ioctl(-1, VIDIOC_QUERYCAP, NULL);
		printf("ioctl(-1, %s, NULL) = %s%s\n",
		       XLAT_STR(VIDIOC_QUERYCAP), sprintrc(rc),
		       rc == inject_retval ? " (INJECTED)" : "");

		if (rc != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", VIDIOC_QUERYCAP, NULL) returning %lu",
				   inject_retval);


	/* VIDIOC_QUERYCAP */
	struct v4l2_capability *caps = tail_alloc(sizeof(*caps));

	fill_memory(caps, sizeof(*caps));
	caps->capabilities = 0xdeadbeef;
#ifdef HAVE_STRUCT_V4L2_CAPABILITY_DEVICE_CAPS
	caps->device_caps = 0xfacefeed;
#endif

	ioctl(-1, VIDIOC_QUERYCAP, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_QUERYCAP), inject_retval);

	ioctl(-1, VIDIOC_QUERYCAP, (char *) caps + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_QUERYCAP), (char *) caps + 1, inject_retval);

	ioctl(-1, VIDIOC_QUERYCAP, caps);
	printf("ioctl(-1, %s, {driver=", XLAT_STR(VIDIOC_QUERYCAP));
	print_quoted_cstring((char *) caps->driver, sizeof(caps->driver));
	printf(", card=");
	print_quoted_cstring((char *) caps->card, sizeof(caps->card));
	printf(", bus_info=");
	print_quoted_cstring((char *) caps->bus_info, sizeof(caps->bus_info));
	printf(", version="
#ifdef WORDS_BIGENDIAN
	       XLAT_KNOWN(0xd0d1d2d3, "KERNEL_VERSION(53457, 210, 211)")
#else
	       XLAT_KNOWN(0xd3d2d1d0, "KERNEL_VERSION(54226, 209, 208)")
#endif
	       ", capabilities=" XLAT_KNOWN(0xdeadbeef,
	       "V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_VIDEO_OUTPUT"
	       "|V4L2_CAP_VIDEO_OVERLAY|V4L2_CAP_VBI_OUTPUT"
	       "|V4L2_CAP_SLICED_VBI_CAPTURE|V4L2_CAP_SLICED_VBI_OUTPUT"
	       "|V4L2_CAP_VIDEO_OUTPUT_OVERLAY|V4L2_CAP_HW_FREQ_SEEK"
	       "|V4L2_CAP_RDS_OUTPUT|V4L2_CAP_VIDEO_CAPTURE_MPLANE"
	       "|V4L2_CAP_VIDEO_OUTPUT_MPLANE|V4L2_CAP_VIDEO_M2M"
	       "|V4L2_CAP_TUNER|V4L2_CAP_RADIO|V4L2_CAP_MODULATOR"
	       "|V4L2_CAP_EXT_PIX_FORMAT|V4L2_CAP_META_CAPTURE|V4L2_CAP_ASYNCIO"
	       "|V4L2_CAP_STREAMING|V4L2_CAP_META_OUTPUT|V4L2_CAP_TOUCH"
	       "|V4L2_CAP_DEVICE_CAPS|0x40000008"));
#ifdef HAVE_STRUCT_V4L2_CAPABILITY_DEVICE_CAPS
	printf(", device_caps=" XLAT_KNOWN(0xfacefeed,
	       "V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_VIDEO_OVERLAY"
	       "|V4L2_CAP_VBI_OUTPUT|V4L2_CAP_SLICED_VBI_CAPTURE"
	       "|V4L2_CAP_SLICED_VBI_OUTPUT|V4L2_CAP_VIDEO_OUTPUT_OVERLAY"
	       "|V4L2_CAP_HW_FREQ_SEEK|V4L2_CAP_RDS_OUTPUT"
	       "|V4L2_CAP_VIDEO_CAPTURE_MPLANE|V4L2_CAP_VIDEO_OUTPUT_MPLANE"
	       "|V4L2_CAP_VIDEO_M2M_MPLANE|V4L2_CAP_VIDEO_M2M|V4L2_CAP_AUDIO"
	       "|V4L2_CAP_RADIO|V4L2_CAP_MODULATOR|V4L2_CAP_SDR_OUTPUT"
	       "|V4L2_CAP_META_CAPTURE|V4L2_CAP_ASYNCIO|V4L2_CAP_META_OUTPUT"
	       "|V4L2_CAP_TOUCH|V4L2_CAP_DEVICE_CAPS|0x60000008"));
#endif
	printf("}) = %ld (INJECTED)\n", inject_retval);


	/* VIDIOC_ENUM_FMT */
	static const struct strval32 buf_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_BUF_TYPE_???") },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_BUF_TYPE_VIDEO_CAPTURE") },
		{ ARG_XLAT_KNOWN(0x3, "V4L2_BUF_TYPE_VIDEO_OVERLAY") },
		{ ARG_XLAT_KNOWN(0x5, "V4L2_BUF_TYPE_VBI_OUTPUT") },
		{ ARG_XLAT_KNOWN(0x6, "V4L2_BUF_TYPE_SLICED_VBI_CAPTURE") },
		{ ARG_XLAT_KNOWN(0xa, "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE") },
		{ ARG_XLAT_KNOWN(0xb, "V4L2_BUF_TYPE_SDR_CAPTURE") },
		{ ARG_XLAT_KNOWN(0xe, "V4L2_BUF_TYPE_META_OUTPUT") },
		{ ARG_XLAT_UNKNOWN(0x80, "V4L2_BUF_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xbadc0ded, "V4L2_BUF_TYPE_???") },
	};
	static const struct strval32 fmtdesc_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_FMT_FLAG_COMPRESSED") },
		{ ARG_XLAT_KNOWN(0x1e, "V4L2_FMT_FLAG_EMULATED"
				       "|V4L2_FMT_FLAG_CONTINUOUS_BYTESTREAM"
				       "|V4L2_FMT_FLAG_DYN_RESOLUTION|0x10") },
		{ ARG_XLAT_UNKNOWN(0xdead0000, "V4L2_FMT_FLAG_???") },
	};
	static const struct strval32 fmtdesc_fmts[] = {
		{ 0x4c47504a, RAW("0x4c47504a")
			      NRAW("v4l2_fourcc('J', 'P', 'G', 'L')"
			           " /* V4L2_PIX_FMT_JPGL */") },
		{ 0xbadc0ded, RAW("0xbadc0ded")
			      NRAW("v4l2_fourcc('\\xed', '\\x0d', '\\xdc',"
			           " '\\xba')") },
	};
	struct v4l2_fmtdesc *fmtdesc = tail_alloc(sizeof(*fmtdesc));

	fill_memory(fmtdesc, sizeof(*fmtdesc));
	fmtdesc->index = 0xdeac0de;

	ioctl(-1, VIDIOC_ENUM_FMT, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FMT), inject_retval);

	ioctl(-1, VIDIOC_ENUM_FMT, (char *) fmtdesc + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_ENUM_FMT), (char *) fmtdesc + 1, inject_retval);

	for (size_t i = 0; i < ARRAY_SIZE(buf_types); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(fmtdesc_flags); j++) {
			for (size_t k = 0; k < ARRAY_SIZE(fmtdesc_fmts); k++) {
				fmtdesc->type = buf_types[i].val;
				fmtdesc->flags = fmtdesc_flags[j].val;
				fmtdesc->pixelformat = fmtdesc_fmts[k].val;

				ioctl(-1, VIDIOC_ENUM_FMT, fmtdesc);
				printf("ioctl(-1, %s, {index=233488606, type=%s"
				       ", flags=%s, description=",
				       XLAT_STR(VIDIOC_ENUM_FMT),
				       buf_types[i].str,
				       fmtdesc_flags[j].str);
				print_quoted_cstring((char *) fmtdesc->description,
					sizeof(fmtdesc->description));
				printf(", pixelformat=%s}) = %ld (INJECTED)\n",
				       fmtdesc_fmts[k].str, inject_retval);

				fill_memory_ex(fmtdesc->description,
					       sizeof(fmtdesc->description),
					       (i * 9 + j) * 7 + k,
					       (k * 3 + j) * 11 + i + 5);
			}
		}
	}


	/* VIDIOC_REQBUFS */
	static const struct strval32 reqb_mems[] = {
		{ ARG_XLAT_UNKNOWN(0, "V4L2_MEMORY_???") },
		{ ARG_XLAT_KNOWN(0x1, "V4L2_MEMORY_MMAP") },
		{ ARG_XLAT_KNOWN(0x4, "V4L2_MEMORY_DMABUF") },
		{ ARG_XLAT_UNKNOWN(0x5, "V4L2_MEMORY_???") },
		{ ARG_XLAT_UNKNOWN(0xbadc0ded, "V4L2_MEMORY_???") },
	};
	struct v4l2_requestbuffers *reqb = tail_alloc(sizeof(*reqb));

	fill_memory(reqb, sizeof(*reqb));
	reqb->count = 0xfeedface;

	ioctl(-1, VIDIOC_REQBUFS, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_REQBUFS), inject_retval);

	ioctl(-1, VIDIOC_REQBUFS, (char *) reqb + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_REQBUFS), (char *) reqb + 1, inject_retval);

	for (size_t i = 0; i < MAX(ARRAY_SIZE(buf_types),
				   ARRAY_SIZE(reqb_mems)); i++) {
		reqb->type = buf_types[i % ARRAY_SIZE(buf_types)].val;
		reqb->memory = reqb_mems[i % ARRAY_SIZE(reqb_mems)].val;

		ioctl(-1, VIDIOC_REQBUFS, reqb);
		printf("ioctl(-1, %s, {type=%s, memory=%s"
		       ", count=4277009102 => 4277009102}) = %ld (INJECTED)\n",
		       XLAT_STR(VIDIOC_REQBUFS),
		       buf_types[i % ARRAY_SIZE(buf_types)].str,
		       reqb_mems[i % ARRAY_SIZE(reqb_mems)].str,
		       inject_retval);
	}


	/* VIDIOC_G_FMT, VIDIOC_S_FMT, VIDIOC_TRY_FMT */
	/*static const struct strval32 fmt_cmds[] = {
	};

	struct v4l2_format fmt = tail_alloc(sizeof(*fmt));

	fill_memory(fmt, sizeof(*fmt));

	for (size_t cmd = 0; cmd < ARRAY_SIZE(fmt_cmds); cmd++)
	ioctl(-1, VIDIOC_G_FMT, 0);
	printf("ioctl(-1, %s, NULL) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_G_FMT), inject_retval);

	ioctl(-1, VIDIOC_G_FMT, (char *) fmt + 1);
	printf("ioctl(-1, %s, %p) = %ld (INJECTED)\n",
	       XLAT_STR(VIDIOC_G_FMT), (char *) fmt + 1, inject_retval);
*/

	puts("+++ exited with 0 +++");

	return 0;
}
