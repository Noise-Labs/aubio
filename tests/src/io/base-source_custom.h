// this should be included *after* custom functions have been defined

#ifndef aubio_source_custom
#define aubio_source_custom "undefined"
#endif /* aubio_source_custom */

#ifdef HAVE_AUBIO_SOURCE_CUSTOM
int test_wrong_params(void);

int base_main(int argc, char **argv)
{
  uint_t err = 0;
  if (argc < 2) {
    PRINT_ERR("not enough arguments, running tests\n");
    err = test_wrong_params();
    PRINT_MSG("read a wave file as a mono vector\n");
    PRINT_MSG("usage: %s <source_path> [samplerate] [hop_size]\n", argv[0]);
    PRINT_MSG("examples:\n");
    PRINT_MSG(" - read file.wav at original samplerate\n");
    PRINT_MSG("       %s file.wav\n", argv[0]);
    PRINT_MSG(" - read file.wav at 32000Hz\n");
    PRINT_MSG("       %s file.aif 32000\n", argv[0]);
    PRINT_MSG(" - read file.wav at original samplerate with 4096 blocks\n");
    PRINT_MSG("       %s file.wav 0 4096 \n", argv[0]);
    return err;
  }

  uint_t samplerate = 0;
  uint_t hop_size = 256;
  uint_t n_frames = 0, read = 0;
  if ( argc >= 3 ) samplerate = atoi(argv[2]);
  if ( argc >= 4 ) hop_size = atoi(argv[3]);

  char_t *source_path = argv[1];

  aubio_source_custom_t * s =
    new_aubio_source_custom(source_path, samplerate, hop_size);
  fvec_t *vec = new_fvec(hop_size);
  if (!s || !vec) { err = 1; goto beach; }

  uint_t n_frames_expected = aubio_source_custom_get_duration(s);

  samplerate = aubio_source_custom_get_samplerate(s);

  do {
    aubio_source_custom_do(s, vec, &read);
    fvec_print (vec);
    n_frames += read;
  } while ( read == hop_size );

  PRINT_MSG("read %d frames (expected %d) at %dHz (%d blocks) from %s\n",
            n_frames, n_frames_expected, samplerate, n_frames / hop_size,
            source_path);

  // close the file (optional)
  aubio_source_custom_close(s);

beach:
  if (vec)
    del_fvec(vec);
  if (s)
    del_aubio_source_custom(s);
  return err;
}

int test_wrong_params(void)
{
  char_t *uri = DEFINEDSTRING(AUBIO_TESTS_SOURCE);
  uint_t samplerate = 44100;
  uint_t hop_size = 512;
  uint_t channels, read = 0;
  fvec_t *vec;
  fmat_t *mat;
  aubio_source_custom_t *s;

  if (new_aubio_source_custom(0,    samplerate, hop_size)) return 1;
  if (new_aubio_source_custom("\0", samplerate, hop_size)) return 1;
  if (new_aubio_source_custom(uri,          -1, hop_size)) return 1;
  if (new_aubio_source_custom(uri,           0,        0)) return 1;

  s = new_aubio_source_custom(uri, samplerate, hop_size);
  if (!s) return 1;
  channels = aubio_source_custom_get_channels(s);

  // vector to read downmixed samples
  vec = new_fvec(hop_size);
  // matrix to read individual channels
  mat = new_fmat(channels, hop_size);

  if (aubio_source_custom_get_samplerate(s) != samplerate) return 1;

  // read first hop_size frames
  aubio_source_custom_do(s, vec, &read);
  if (read != hop_size) return 1;

  // read again in undersized vector
  del_fvec(vec);
  vec = new_fvec(hop_size - 1);
  aubio_source_custom_do(s, vec, &read);
  if (read != hop_size - 1) return 1;

  // read again in oversized vector
  del_fvec(vec);
  vec = new_fvec(hop_size + 1);
  aubio_source_custom_do(s, vec, &read);
  if (read != hop_size) return 1;

  // seek to 0
  if(aubio_source_custom_seek(s, 0)) return 1;

  // read again as multiple channels
  aubio_source_custom_do_multi(s, mat, &read);
  if (read != hop_size) return 1;

  // read again as multiple channels in an undersized matrix
  del_fmat(mat);
  mat = new_fmat(channels - 1, hop_size);
  aubio_source_custom_do_multi(s, mat, &read);
  if (read != hop_size) return 1;

  // read again as multiple channels in an undersized matrix
  del_fmat(mat);
  mat = new_fmat(channels, hop_size - 1);
  aubio_source_custom_do_multi(s, mat, &read);
  if (read != hop_size - 1) return 1;

  // read again as multiple channels in an oversized matrix
  del_fmat(mat);
  mat = new_fmat(channels + 1, hop_size);
  aubio_source_custom_do_multi(s, mat, &read);
  if (read != hop_size) return 1;

  // read again as multiple channels in an oversized matrix
  del_fmat(mat);
  mat = new_fmat(channels, hop_size + 1);
  aubio_source_custom_do_multi(s, mat, &read);
  if (read != hop_size) return 1;

  // close the file (optional)
  aubio_source_custom_close(s);
  // test closing the file a second time
  aubio_source_custom_close(s);

  del_aubio_source_custom(s);
  del_fmat(mat);
  del_fvec(vec);

  return run_on_default_source(base_main);
}

#else /* HAVE_AUBIO_SOURCE_CUSTOM */

int base_main(int argc, char** argv)
{
  PRINT_ERR("aubio was not compiled with aubio_source_"
          aubio_source_custom ", failed running %s with %d args\n",
          argv[0], argc);
  return 0;
}

#endif /* HAVE_AUBIO_SOURCE_CUSTOM */