#include <napi.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <as400_protos.h>
#include <as400_types.h>
using namespace Napi;

#include <ctype.h>

typedef struct
{
  int bytes_provided;
  int bytes_available;
  char msgid[7];
  char reserved[1];
  char exc_data[100];
} error_info_t __attribute__((packed));
int to_37_spacepadded_nts(char *out, size_t out_len, const char *in)
{
  size_t in_len = strlen(in);
  char in_buf[out_len];
  in_buf[-1 + out_len] = 0;
  memset(in_buf, ' ', -1 + out_len);

  for (int i = 0; i < in_len; i++)
  {
    if (0 == in[i])
      break;
    in_buf[i] = toupper(in[i]);
  }
  iconv_t cd = iconv_open("IBM-037", "ISO8859-1");
  if ((iconv_t)-1 == cd)
  {
    fprintf(stderr, "Error in opening conversion descriptors\n");
    return 8;
  }

  size_t inleft = strlen(in_buf);
  size_t outleft = out_len;
  char *input = in_buf;
  char *output = out;

  int rc = iconv(cd, &input, &inleft, &output, &outleft);
  if (rc == -1)
  {
    fprintf(stderr, "Error in converting characters\n");
    return 9;
  }
  return iconv_close(cd);
}

int qsygetph(const char *_username, const char *_pw, char *_handle_buf)
{
  if (0 == _pw)
  {
    return -1;
  }
  char usrprf_ebcdic[11] __attribute__((aligned(16)));
  to_37_spacepadded_nts(usrprf_ebcdic, 11, _username);
  char handle[12] __attribute__((aligned(16)));
  memset(handle, 0, sizeof(handle));

  static ILEpointer qsygetph_pgm __attribute__((aligned(16)));
  static int qsygetph_pgm_loaded = 0;
  int rc = -1;
  if (0 == qsygetph_pgm_loaded)
  {
    rc = _RSLOBJ2(&qsygetph_pgm,
                  RSLOBJ_TS_PGM,
                  "QSYGETPH",
                  "QSYS");
    if (0 != rc)
    {
      return rc;
    }
    qsygetph_pgm_loaded = 1;
  }

  error_info_t errinfo __attribute__((aligned(16), packed));
  memset(&errinfo, 0, sizeof(errinfo));
  errinfo.bytes_provided = sizeof(errinfo);

  int ccsid = _SETCCSID(-1);
  int pwlen = strlen(_pw);
  void *pgm_argv[] __attribute__((aligned(16))) = {
      &usrprf_ebcdic,
      (void *)_pw,
      &handle,
      &errinfo,
      &pwlen,
      &ccsid,
      NULL};
  rc = _PGMCALL(&qsygetph_pgm,
                pgm_argv,
                PGMCALL_EXCP_NOSIGNAL);
  if (0 == rc && 0 == errinfo.bytes_available)
  {
    if (0 != _handle_buf)
    {
      memcpy(_handle_buf, handle, 12);
    }
    return 0;
  }
  else
  {
    if (0 != _handle_buf)
    {
      *_handle_buf = 0;
    }
    return -1;
  }
}
int qsyrlsph(char *_handle_buf)
{
  static ILEpointer qsyrlsph_pgm __attribute__((aligned(16)));
  static int qsyrlsph_pgm_loaded = 0;
  int rc = -1;
  if (0 == qsyrlsph_pgm_loaded)
  {
    rc = _RSLOBJ2(&qsyrlsph_pgm,
                  RSLOBJ_TS_PGM,
                  "QSYRLSPH",
                  "QSYS");
    if (0 != rc)
    {
      return rc;
    }
    qsyrlsph_pgm_loaded = 1;
  }
  error_info_t errinfo __attribute__((aligned(16), packed));
  memset(&errinfo, 0, sizeof(errinfo));
  errinfo.bytes_provided = sizeof(errinfo);
  char *handle = _handle_buf;
  void *pgm_argv[] __attribute__((aligned(16))) = {
      &handle,
      &errinfo,
      NULL};
  rc = _PGMCALL(&qsyrlsph_pgm,
                pgm_argv,
                PGMCALL_EXCP_NOSIGNAL);
  if (0 == rc && 0 == errinfo.bytes_available)
  {
    return 0;
  }
  return -1;
}

int isSpecialValue(const char *_pw)
{
  if (_pw == strstr(_pw, "*NOPWD"))
  {
    return 1;
  }
  return 0;
}
int validate_pw0(const char *_username, const char *_pw)
{
  if (0 == _pw)
  {
    return -1;
  }
  if (isSpecialValue(_pw))
  {
    return -1;
  }
  char handle[12];
  int rc = qsygetph(_username, _pw, handle);
  if (0 == rc)
  {
    int rc2 = qsyrlsph(handle);
  }
  return rc;
}
int validate_pw(std::string _username, std::string _password)
{
  int rc = validate_pw0(_username.c_str(), _password.c_str());
  return rc;
}

Napi::String Method(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  return Napi::String::New(env, "world");
}

Napi::Boolean doVerify(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::String username = info[0].ToString();
  Napi::String password = info[1].ToString();
  return Napi::Boolean::New(env, 0 == validate_pw(username.Utf8Value(), password.Utf8Value()));
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "verifyIbmiPw"),
              Napi::Function::New(env, doVerify));
  return exports;
}

NODE_API_MODULE(addon, Init)