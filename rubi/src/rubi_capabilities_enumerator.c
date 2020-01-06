
#include "rubi.h"
#include "rubi_auxiliary.h"
#include "rubi_internals.h"
#include "rubi_protocol.h"

uint32_t rubi_relaytable_size;

extern const char __rubi_board_name[];
extern const char __rubi_board_version[];
extern const char __rubi_board_driver[];
extern const char __rubi_board_desc[];
extern const uint16_t __rubi_update_hz;

extern char *__rubi_board_id;

extern const uint32_t __rubi_decl_count;
extern uint32_t __rubi_crctable[];
extern int8_t __rubi_typetable[];

static uint32_t enumeration_cursor;

uint32_t rubi_field_crc(int32_t id)
{
    rubi_ffdescriptor_t *descriptor = (*__rubi_fieldescfunction_getters[id])();
    RUBI_ASSERT(descriptor->fftype == __RUBI_FIELD_MAGIC);

    const uint8_t *relayfield =
        descriptor->descriptor.field_descriptor.relayfield;
    switch (descriptor->descriptor.field_descriptor.size)
    {
    case 0:
        RUBI_ASSERT(0);
        return 0;
    case 1:
        return *relayfield;
    case 2:
        return *((uint16_t *)relayfield);
    case 3:
        return (*(relayfield)) + (*(relayfield + 1) << 8) +
               (*(relayfield + 1) << 16);
    case 4:
        return *((uint32_t *)relayfield);
    default:
        return rubi_crc_nopoly(
            (uint32_t *)relayfield,
            (descriptor->descriptor.field_descriptor.size + 3) / 4);
    }
}

uint8_t rubi_get_field_size(uint32_t id)
{
    rubi_ffdescriptor_t *descriptor = (*__rubi_fieldescfunction_getters[id])();
    RUBI_ASSERT(descriptor->fftype == __RUBI_FIELD_MAGIC);

    return descriptor->descriptor.field_descriptor.size;
}

uint8_t rubi_get_field_access(uint32_t id)
{
    rubi_ffdescriptor_t *descriptor = (*__rubi_fieldescfunction_getters[id])();
    RUBI_ASSERT(descriptor->fftype == __RUBI_FIELD_MAGIC);

    return descriptor->descriptor.field_descriptor.access;
}

uint32_t *rubi_field_update_crc(uint32_t id,
                                uint32_t size __attribute__((unused)))
{
    rubi_ffdescriptor_t *descriptor = (*__rubi_fieldescfunction_getters[id])();
    RUBI_ASSERT(descriptor->fftype == __RUBI_FIELD_MAGIC);

    uint32_t old_crc = __rubi_crctable[id];
    __rubi_crctable[id] = rubi_field_crc(id);

    return old_crc == __rubi_crctable[id]
               ? NULL
               : (uint32_t *)descriptor->descriptor.field_descriptor.relayfield;
}

void rubi_write_ffdata(uint8_t id, uint8_t *ptr)
{
    rubi_ffdescriptor_t *descriptor = (*__rubi_fieldescfunction_getters[id])();
    RUBI_ASSERT(descriptor->fftype == __RUBI_FIELD_MAGIC);

    memcpy((void *)descriptor->descriptor.field_descriptor.relayfield, ptr,
           descriptor->descriptor.field_descriptor.size);
}

void rubi_capabilities_enumeration_start()
{
    rubi_send_infotext(RUBI_INFO_BOARD_NAME, __rubi_board_name,
                       strlen(__rubi_board_name));
    rubi_send_infotext(RUBI_INFO_BOARD_VERSION, __rubi_board_version,
                       strlen(__rubi_board_version));
    rubi_send_infotext(RUBI_INFO_BOARD_DRIVER, __rubi_board_driver,
                       strlen(__rubi_board_driver));
    rubi_send_infotext(RUBI_INFO_BOARD_DESC, __rubi_board_desc,
                       strlen(__rubi_board_desc));

    if (__rubi_board_id != NULL)
    {
        size_t board_id_len = strlen(__rubi_board_id);
        rubi_send_infotext(RUBI_INFO_BOARD_ID, __rubi_board_id, board_id_len);
    }

    enumeration_cursor = 0;
}

bool_t rubi_capabilities_enumeration_continue()
{
    rubi_ffdescriptor_t *descriptor;

    if (enumeration_cursor == __rubi_decl_count)
        return false;

    descriptor = (*__rubi_fieldescfunction_getters[enumeration_cursor])();

    {
        const rubi_field_desc_t *field_dsc =
            &descriptor->descriptor.field_descriptor;
        switch (descriptor->fftype)
        {
        case __RUBI_FIELD_MAGIC:
            rubi_send_infotext(RUBI_INFO_FIELD_NAME, field_dsc->name,
                               strlen(field_dsc->name));

            rubi_send_infotext(RUBI_INFO_SUBFIELDS, field_dsc->subnames,
                               strlen(field_dsc->subnames));

            rubi_send_infonumber(RUBI_INFO_FIELD_ACCESS, field_dsc->access);

            rubi_send_infonumber(RUBI_INFO_FIELD_TYPE, field_dsc->type);

            rubi_send_infonumber(RUBI_INFO_SUBFIELDS_C,
                                 field_dsc->size /
                                     rubi_type_size(field_dsc->type));

            __rubi_typetable[enumeration_cursor] = (uint8_t)__RUBI_FIELD_MAGIC;
            break;
        case __RUBI_FUNC_MAGIC:
        default:
            RUBI_ASSERT(0);
        };
    }

    ++enumeration_cursor;
    return true;
}
