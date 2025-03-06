#include "main_task.h"

// Queue handle used to manipulate the main queue of events
static QueueHandle_t main_task_queue_handle;

BaseType_t main_task_send_message(main_task_message_e msgID)
{
    main_task_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(main_task_queue_handle, &msg, portMAX_DELAY);
}

BaseType_t main_task_send_message_from_isr(main_task_message_e msgID)
{
    main_task_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSendFromISR(main_task_queue_handle, &msg, NULL);
}

void main_task_init(void)
{
    main_task_queue_handle = xQueueCreate(200, sizeof(main_task_queue_message_t));
    xTaskCreatePinnedToCore(&main_task, "main_task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL, MAIN_TASK_CORE_ID);
}

void main_task(void *pvParameters)
{
    cout << "Main task started" << endl;
    main_task_queue_message_t msg;
    unsigned char sn[SN_SIZE] = {'S', '1', 'm', 'C', '0', 'M', '5'};
    pins_init();
    Simcom simcom = Simcom(sn);
    simcom.power(true);

    for (;;)
    {
        if (xQueueReceive(main_task_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {

            case START_MQTT:
                simcom.set_queue(create_start_mqtt_queue());
                simcom.send();
                break;

            case SEND_MSG:
                simcom.set_queue(create_send_msg_queue());
                simcom.send();
                break;

            case RESTART_DEVICE:
                simcom.power(false);
                esp_restart();
                break;

            case UART_DATA_RECEIVED:
                break;

            case SS_ISR:
                break;

            default:
                break;
            }
        }
    }
}

SimcomCmdQueue create_start_mqtt_queue()
{
    SimcomCmdQueue queue = SimcomCmdQueue(cmd_queue_type_e::PDN_AUTO_ACT);

    Command cmd = Command(CPIN, CMD_action_enum::READ);
    queue.enqueue(cmd);

    cmd = Command(CSQ, CMD_action_enum::EXE);
    queue.enqueue(cmd);

    cmd = Command(CGATT, CMD_action_enum::READ);
    queue.enqueue(cmd);

    cmd = Command(COPS, CMD_action_enum::READ);
    queue.enqueue(cmd);

    cmd = Command(CGNAPN, CMD_action_enum::EXE);
    queue.enqueue(cmd);

    cmd = Command(CNCFG, CMD_action_enum::WRITE);
    cmd.add_value(Value((int)0));
    cmd.add_value(Value((int)1));
    cmd.add_value(Value("ctnb"));
    queue.enqueue(cmd);

    cmd = Command(CNACT, CMD_action_enum::WRITE);
    cmd.add_value(Value((int)0));
    cmd.add_value(Value((int)1));
    queue.enqueue(cmd);

    cmd = Command(CNACT, CMD_action_enum::READ);
    queue.enqueue(cmd);

    return queue;
}

SimcomCmdQueue create_send_msg_queue()
{
    SimcomCmdQueue queue = SimcomCmdQueue(cmd_queue_type_e::EMPTY);
    return queue;
}