#include "QuicConfig.h"

#include <assert.h>

using namespace baize;

net::QuicConfig::QuicConfig(uint32_t version)
    : config_(quiche_config_new(version)) {
  assert(config_ != nullptr);
}

net::QuicConfig::~QuicConfig() { quiche_config_free(config_); }

void net::QuicConfig::setCertAndKey(const char *cert, const char *key) {
  quiche_config_load_cert_chain_from_pem_file(config_, cert);
  quiche_config_load_priv_key_from_pem_file(config_, key);
}

void net::QuicConfig::setClientConfig() {
  uint8_t proto[] = "\x0ahq-interop\x05hq-29\x05hq-28\x05hq-27\x08http/0.9";
  quiche_config_set_application_protos(config_, proto, 38);
  quiche_config_set_max_idle_timeout(config_, 5000);
  quiche_config_set_max_recv_udp_payload_size(config_, kMaxDatagramSize);
  quiche_config_set_max_send_udp_payload_size(config_, kMaxDatagramSize);
  quiche_config_set_initial_max_data(config_, 10000000);
  quiche_config_set_initial_max_stream_data_bidi_local(config_, 1000000);
  quiche_config_set_initial_max_stream_data_uni(config_, 1000000);
  quiche_config_set_initial_max_streams_bidi(config_, 100);
  quiche_config_set_initial_max_streams_uni(config_, 100);
  quiche_config_set_disable_active_migration(config_, true);
}

void net::QuicConfig::setServerConfig() {
  uint8_t proto[] = "\x0ahq-interop\x05hq-29\x05hq-28\x05hq-27\x08http/0.9";
  quiche_config_set_application_protos(config_, proto, 38);
  quiche_config_set_max_idle_timeout(config_, 5000);
  quiche_config_set_max_recv_udp_payload_size(config_, kMaxDatagramSize);
  quiche_config_set_max_send_udp_payload_size(config_, kMaxDatagramSize);
  quiche_config_set_initial_max_data(config_, 10000000);
  quiche_config_set_initial_max_stream_data_bidi_local(config_, 1000000);
  quiche_config_set_initial_max_stream_data_bidi_remote(config_, 1000000);
  quiche_config_set_initial_max_streams_bidi(config_, 100);
  quiche_config_set_cc_algorithm(config_, QUICHE_CC_RENO);
}