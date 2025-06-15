#include "zed.h"
#include "sens.h"
#include "host.h"

// �������� �������� ��� ��������
#define SENS_TEMP_SCALE     100

// �������� ��������� ����������� [C]
const int16_t SENS_TEMP_RANGE_MIN = -40 * SENS_TEMP_SCALE,
              SENS_TEMP_RANGE_MAX = +80 * SENS_TEMP_SCALE;

// �������� ��������� ����������� [C] (0.01)
const uint16_t SENS_TEMP_TOLERANCE = 1;

// ������� �������� �����������
int16_t sens_temp_current = 25 * SENS_TEMP_SCALE;

// ���������� ���������� �����������
static void sens_temp_ready(const uint8_t *data)
{
    // �������
    memcpy(&sens_temp_current, data, sizeof(sens_temp_current));
    
    // ����������
    zed_attr_temp_changed();
}

// ������ �������� ���������� (������ ������)
uint8_t sens_bat_volatage = 30;
// ������ �������� �������� ������������� (������ ������)
uint8_t sens_bat_precentage = 160;

// ���������� ���������� ���������� �������
static void sens_bat_ready(const uint8_t *data)
{
    // ����� �������� [��]
    uint16_t mv;
    memcpy(&mv, data, sizeof(mv));
    
    // ������ ���������� [��]
    const uint16_t TH_LOW = 2800;
    const uint16_t TH_HIGH = 3600;
    const uint16_t TH_DELTA = TH_HIGH - TH_LOW;
    
    // �������� ����������
    sens_bat_volatage = (uint8_t)(mv / 100);
    
    // ������������ ��� ��������
    if (mv < TH_LOW)
        mv = TH_LOW;
    if (mv > TH_HIGH)
        mv = TH_HIGH;
    mv -= TH_LOW;
    
    // ������ ��������
    const uint32_t MAX_PRECENT = 200;
    sens_bat_precentage = (MAX_PRECENT * mv / TH_DELTA);
    
    // ��������� �������
    static uint8_t prescaler = 0;
    if (prescaler > 0)
    {
        prescaler--;
        return;
    }
    prescaler = 10;
    
    // ����������
    zed_attr_bat_changed();
}

void sens_init(void)
{
    static const host_req_t REQ_LIST[] =
    {
        { 0x0008, sens_temp_ready },
        { 0x000A, sens_bat_ready },
    };
    
    host_req_list_set(REQ_LIST, ARRAY_SIZE(REQ_LIST));
}