/*
 * SDL_sound -- An abstract sound format decoding API.
 * Copyright (C) 2001  Ryan C. Gordon.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * RAW decoder for SDL_sound. This is as simple as it gets.
 *
 * This driver handles raw audio data. You must, regardless of where the
 *  data is actually coming from, specify the string "RAW" in the extension
 *  parameter of Sound_NewSample() (or, alternately, open a file with the
 *  extension ".raw" in Sound_NewSampleFromFile()). The string is checked
 *  case-insensitive. We need this check, because raw data, being raw, has
 *  no headers or magic number we can use to determine if we should handle a
 *  given file, so we needed some way to have this "decoder" discriminate.
 *
 * When calling Sound_NewSample*(), you must also specify a "desired"
 *  audio format. The "actual" format will always match what you specify, so
 *  there will be no conversion overhead, but these routines need to know how
 *  to treat the bits, since it's all random garbage otherwise.
 *
 * Please see the file LICENSE in the source's root directory.
 *
 *  This file written by Ryan C. Gordon. (icculus@clutteredmind.org)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "SDL_sound.h"

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if (!defined SOUND_SUPPORTS_RAW)
#error SOUND_SUPPORTS_RAW must be defined.
#endif


static int RAW_init(void);
static void RAW_quit(void);
static int RAW_open(Sound_Sample *sample, const char *ext);
static void RAW_close(Sound_Sample *sample);
static Uint32 RAW_read(Sound_Sample *sample);

const Sound_DecoderFunctions __Sound_DecoderFunctions_RAW =
{
    {
        "RAW",
        "Raw audio",
        "Ryan C. Gordon <icculus@clutteredmind.org>",
        "http://www.icculus.org/SDL_sound/"
    },

    RAW_init,       /*  init() method */
    RAW_quit,       /*  quit() method */
    RAW_open,       /*  open() method */
    RAW_close,      /* close() method */
    RAW_read        /*  read() method */
};


static int RAW_init(void)
{
    return(1);  /* always succeeds. */
} /* RAW_init */


static void RAW_quit(void)
{
    /* it's a no-op. */
} /* RAW_quit */


static int RAW_open(Sound_Sample *sample, const char *ext)
{
        /*
         * We check this explicitly, since we have no other way to
         *  determine whether we should handle this data or not.
         */
    if (__Sound_strcasecmp(ext, "RAW") != 0)
    {
        Sound_SetError("RAW: extension isn't explicitly \"RAW\".");
        return(0);
    } /* if */

        /*
         * You must also specify a desired format, so we know how to
         *  treat the bits that are otherwise binary garbage.
         */
    if ( (sample->desired.channels < 1)  ||
         (sample->desired.channels > 2)  ||
         (sample->desired.rate == 0)     ||
         (sample->desired.format == 0) )
    {
        Sound_SetError("RAW: invalid desired format.");
        return(0);
    } /* if */

    _D(("RAW: Accepting data stream.\n"));

        /*
         * We never convert raw samples; what you ask for is what you get.
         */
    memcpy(&sample->actual, &sample->desired, sizeof (Sound_AudioInfo));
    sample->flags = SOUND_SAMPLEFLAG_NONE;

    return(1); /* we'll handle this data. */
} /* RAW_open */


static void RAW_close(Sound_Sample *sample)
{
    /* we don't allocate anything that we need to free. That's easy, eh? */
} /* RAW_close */


static Uint32 RAW_read(Sound_Sample *sample)
{
    Uint32 retval;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

        /*
         * We don't actually do any decoding, so we read the raw data
         *  directly into the internal buffer...
         */
    retval = SDL_RWread(internal->rw, internal->buffer,
                        1, internal->buffer_size);

        /* Make sure the read went smoothly... */
    if (retval == 0)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (retval == -1)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

        /* (next call this EAGAIN may turn into an EOF or error.) */
    else if (retval < internal->buffer_size)
        sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

    return(retval);
} /* RAW_read */


/* end of raw.c ... */

