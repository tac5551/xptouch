#ifndef _XLCD_BBLP
#define _XLCD_BBLP

#ifdef __cplusplus
extern "C"
{
#endif

    bool xtouch_bblp_is_p1p()
    {
        return strcmp(xTouchConfig.xTouchPrinterModel, "C11") == 0;
    }

    bool xtouch_bblp_is_p1s()
    {
        return strcmp(xTouchConfig.xTouchPrinterModel, "C12") == 0;
    }

    bool xtouch_bblp_is_p1Series()
    {
        return xtouch_bblp_is_p1p() || xtouch_bblp_is_p1s();
    }

    bool xtouch_bblp_is_a1()
    {
        const char *m = xTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        if (strcmp(m, "A1") == 0)
            return true;
        return false;
    }

    bool xtouch_bblp_is_a1mini()
    {
        const char *m = xTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        if (strcmp(m, "A1 mini") == 0 || strcmp(m, "A1 Mini") == 0 || strcmp(m, "A1Mini") == 0)
            return true;
        return strstr(m, "A1Mini") != NULL;
    }

    bool xtouch_bblp_is_a1Series()
    {
        const char *m = xTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        return xtouch_bblp_is_a1() || xtouch_bblp_is_a1mini() || (strstr(m, "A1") != NULL);
    }

    bool xtouch_bblp_is_a1p1Series()
    {
        const char *m = xTouchConfig.xTouchPrinterModel;
        if (!m || !m[0])
            return false;
        return xtouch_bblp_is_p1Series() || xtouch_bblp_is_a1Series();
    }

    bool xtouch_bblp_is_x1()
    {
        return strcmp(xTouchConfig.xTouchPrinterModel, "3DPrinter-X1") == 0;
    }

    bool xtouch_bblp_is_x1c()
    {
        return strcmp(xTouchConfig.xTouchPrinterModel, "3DPrinter-X1-Carbon") == 0;
    }

    bool xtouch_bblp_is_x1Series()
    {
        return xtouch_bblp_is_x1() || xtouch_bblp_is_x1c();
    }

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif