#ifndef XPTOUCH_MQTT_CLASS_HPP
#define XPTOUCH_MQTT_CLASS_HPP

/**
 * @file mqtt_class.hpp
 * @brief Bambu 系 MQTT 用の接続単位ラッパ（複数ホスト・複数 PubSubClient を共存させるため）。
 *
 * camera_stream.h と同様、ヘッダのみで完結。既存 mqtt.h のグローバル
 * xptouch_pubSubClient とは別インスタンスとして使う。
 */

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "autogrowstream.h"

namespace xptouch_mqtt
{

class MqttConnection;

namespace detail
{
/** 複数 TU でも ODR 上 1 つ（inline 関数内 static）。loop() からのみ参照する。 */
inline MqttConnection *&loop_context_ptr()
{
    static MqttConnection *p = nullptr;
    return p;
}
} // namespace detail

/** 受信コールバック（PubSubClient が C 関数のみ対応のため userdata で this を渡す） */
using OnMqttMessage = void (*)(void *userdata, char *topic, uint8_t *payload, unsigned int length);

/**
 * 1 接続先 = 1 インスタンス。WiFiClientSecure / 受信バッファ / PubSubClient を束ねる。
 * コピー不可（ソケット・コールバックの二重管理を避ける）。
 */
class MqttConnection
{
public:
    MqttConnection()
        : _pubsub(_wifi)
    {
        _pubsub.setStream(_stream);
        _pubsub.setBufferSize(_buffer_size ? _buffer_size : 2048);
        _pubsub.setCallback(&MqttConnection::s_pubsub_trampoline);
        _pubsub.setKeepAlive(_keep_alive_sec);
    }

    MqttConnection(const MqttConnection &) = delete;
    MqttConnection &operator=(const MqttConnection &) = delete;

    WiFiClientSecure &wifi_client() { return _wifi; }
    PubSubClient &pubsub() { return _pubsub; }
    const PubSubClient &pubsub() const { return _pubsub; }

    const String &request_topic() const { return _request_topic; }
    const String &report_topic() const { return _report_topic; }

    /** device/{serial}/request と device/{serial}/report を組み立てる */
    void set_device_topics(const char *serial)
    {
        if (!serial)
            serial = "";
        String base = String("device/") + serial;
        _request_topic = base + "/request";
        _report_topic = base + "/report";
    }

    /**
     * 受信時に呼ぶ処理を登録。loop() 中だけこのインスタンスがコンテキストとして有効になり userdata 付きで呼ばれる。
     * nullptr で解除。
     */
    void set_message_handler(void *userdata, OnMqttMessage handler)
    {
        _handler_userdata = userdata;
        _on_message = handler;
    }

    /** mqtt.h の xptouch_mqtt_configure_client に相当（TLS は検証しない） */
    void configure_client(const char *host, uint16_t port = 8883)
    {
        _wifi.flush();
        _wifi.stop();
        _wifi.setInsecure();
        _pubsub.setServer(host, port);
        _pubsub.setBufferSize(_buffer_size ? _buffer_size : 2048);
        _pubsub.setStream(_stream);
        _pubsub.setCallback(&MqttConnection::s_pubsub_trampoline);
        _pubsub.setKeepAlive(_keep_alive_sec);
    }

    void set_buffer_size(uint16_t bytes) { _buffer_size = bytes; }
    void set_keep_alive(uint16_t seconds) { _keep_alive_sec = seconds; }

    bool connect(const char *client_id, const char *username, const char *password)
    {
        if (!client_id)
            client_id = "";
        if (!username)
            username = "";
        if (!password)
            password = "";
        return _pubsub.connect(client_id, username, password);
    }

    void disconnect() { _pubsub.disconnect(); }
    bool connected() const { return _pubsub.connected(); }

    int state() const { return _pubsub.state(); }

    bool subscribe_report()
    {
        if (!_report_topic.length())
            return false;
        return _pubsub.subscribe(_report_topic.c_str());
    }

    bool subscribe_request()
    {
        if (!_request_topic.length())
            return false;
        return _pubsub.subscribe(_request_topic.c_str());
    }

    bool subscribe_topic(const char *topic)
    {
        if (!topic || !topic[0])
            return false;
        return _pubsub.subscribe(topic);
    }

    bool publish_request(const char *payload)
    {
        if (!_request_topic.length() || !payload)
            return false;
        return _pubsub.publish(_request_topic.c_str(), payload);
    }

    bool publish(const char *topic, const char *payload, bool retained = false)
    {
        if (!topic || !topic[0] || !payload)
            return false;
        return _pubsub.publish(topic, payload, retained);
    }

    /**
     * 必ずこの入口で loop すること（PubSubClient の生 loop だとコールバックに届かない）。
     * 単スレッド想定。pubsub().loop() を直接呼ぶとコールバックの this 解決が効かないので使わないこと。
     */
    void loop()
    {
        MqttConnection *&slot = detail::loop_context_ptr();
        MqttConnection *prev = slot;
        slot = this;
        _pubsub.loop();
        slot = prev;
    }

private:
    WiFiClientSecure _wifi;
    XtouchAutoGrowBufferStream _stream;
    PubSubClient _pubsub;
    String _request_topic;
    String _report_topic;
    void *_handler_userdata = nullptr;
    OnMqttMessage _on_message = nullptr;
    uint16_t _buffer_size = 2048;
    uint16_t _keep_alive_sec = 10;

    static void s_pubsub_trampoline(char *topic, byte *payload, unsigned int length)
    {
        MqttConnection *self = detail::loop_context_ptr();
        if (!self || !self->_on_message)
            return;
        self->_on_message(self->_handler_userdata, topic, payload, length);
    }
};

} // namespace xptouch_mqtt

#endif // XPTOUCH_MQTT_CLASS_HPP
