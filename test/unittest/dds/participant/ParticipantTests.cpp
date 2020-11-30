// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/core/types.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/topic/Topic.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>


namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::ParticipantAttributes;
using fastrtps::PublisherAttributes;
using fastrtps::SubscriberAttributes;
using fastrtps::xmlparser::XMLProfileManager;
using fastrtps::xmlparser::XMLP_ret;


// Mocked TopicDataType for Topic creation tests
class TopicDataTypeMock : public TopicDataType
{
public:

    TopicDataTypeMock()
        : TopicDataType()
    {
        m_typeSize = 4u;
        setName("footype");
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return std::function<uint32_t()>();
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    void clearName()
    {
        setName("");
    }

};

TEST(ParticipantTests, DomainParticipantFactoryGetInstance)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    ASSERT_NE(factory, nullptr);
    ASSERT_EQ(factory, DomainParticipantFactory::get_instance());
}

TEST(ParticipantTests, ChangeDomainParticipantFactoryQos)
{
    DomainParticipantFactoryQos qos;
    DomainParticipantFactory::get_instance()->get_qos(qos);

    ASSERT_EQ(qos.entity_factory().autoenable_created_entities, true);

    EntityFactoryQosPolicy entity_factory = qos.entity_factory();
    entity_factory.autoenable_created_entities = false;
    qos.entity_factory(entity_factory);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    DomainParticipantFactoryQos fqos;
    DomainParticipantFactory::get_instance()->get_qos(fqos);

    ASSERT_EQ(qos, fqos);
    ASSERT_EQ(fqos.entity_factory().autoenable_created_entities, false);
}

TEST(ParticipantTests, CreateDomainParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_NE(participant, nullptr);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

void check_participant_with_profile (
        DomainParticipant* participant,
        const std::string& profile_name)
{
    DomainParticipantQos qos;
    participant->get_qos(qos);

    ParticipantAttributes participant_atts;
    XMLProfileManager::fillParticipantAttributes(profile_name, participant_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.allocation() == participant_atts.rtps.allocation);
    ASSERT_TRUE(qos.properties() == participant_atts.rtps.properties);
    ASSERT_TRUE(qos.name().to_string() == participant_atts.rtps.getName());
    ASSERT_TRUE(qos.wire_protocol().prefix == participant_atts.rtps.prefix);
    ASSERT_TRUE(qos.wire_protocol().participant_id == participant_atts.rtps.participantID);
    ASSERT_TRUE(qos.wire_protocol().builtin == participant_atts.rtps.builtin);
    ASSERT_TRUE(qos.wire_protocol().port == participant_atts.rtps.port);
    ASSERT_TRUE(qos.wire_protocol().throughput_controller == participant_atts.rtps.throughputController);
    ASSERT_TRUE(qos.wire_protocol().default_unicast_locator_list ==
            participant_atts.rtps.defaultUnicastLocatorList);
    ASSERT_TRUE(qos.wire_protocol().default_multicast_locator_list ==
            participant_atts.rtps.defaultMulticastLocatorList);
    ASSERT_TRUE(qos.transport().user_transports == participant_atts.rtps.userTransports);
    ASSERT_TRUE(qos.transport().use_builtin_transports == participant_atts.rtps.useBuiltinTransports);
    ASSERT_TRUE(qos.transport().send_socket_buffer_size == participant_atts.rtps.sendSocketBufferSize);
    ASSERT_TRUE(qos.transport().listen_socket_buffer_size == participant_atts.rtps.listenSocketBufferSize);
    ASSERT_TRUE(qos.user_data().data_vec() == participant_atts.rtps.userData);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.entity_factory() == PARTICIPANT_QOS_DEFAULT.entity_factory());
}

TEST(ParticipantTests, CreateDomainParticipantWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");

    //participant using the default profile
    DomainParticipant* default_participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(default_participant, nullptr);
    ASSERT_EQ(default_participant->get_domain_id(), 0u); //Keep the DID given to the method, not the one on the profile
    check_participant_with_profile(default_participant, "test_default_participant_profile");
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(
                default_participant) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant_with_profile(0, "test_participant_profile");
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_domain_id(), 0u); //Keep the DID given to the method, not the one on the profile
    check_participant_with_profile(participant, "test_participant_profile");
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetParticipantProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipantQos qos;
    EXPECT_EQ(
        DomainParticipantFactory::get_instance()->get_participant_qos_from_profile("test_participant_profile", qos),
        ReturnCode_t::RETCODE_OK);

    // Extract ParticipantQos from profile
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, qos);
    ASSERT_NE(participant, nullptr);

    check_participant_with_profile(participant, "test_participant_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        DomainParticipantFactory::get_instance()->get_participant_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


TEST(ParticipantTests, CreatePSMDomainParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::core::null;
    participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_NE(participant, ::dds::core::null);
}

TEST(ParticipantTests, DeleteDomainParticipant)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteDomainParticipantWithEntities)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(
                participant), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);

    participant = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(
                participant), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);

    participant = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(
                participant), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangeDefaultParticipantQos)
{
    DomainParticipantQos qos;
    DomainParticipantFactory::get_instance()->get_default_participant_qos(qos);

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    EntityFactoryQosPolicy entity_factory = qos.entity_factory();
    entity_factory.autoenable_created_entities = false;
    qos.entity_factory(entity_factory);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_default_participant_qos(qos) == ReturnCode_t::RETCODE_OK);
    DomainParticipantQos pqos;
    DomainParticipantFactory::get_instance()->get_default_participant_qos(pqos);

    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->set_default_participant_qos(
                PARTICIPANT_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultParticipantQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ::dds::domain::qos::DomainParticipantQos qos = participant.default_participant_qos();

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    EntityFactoryQosPolicy entity_factory = qos.entity_factory();
    entity_factory.autoenable_created_entities = false;
    qos.entity_factory(entity_factory);

    ASSERT_NO_THROW(participant.default_participant_qos(qos));
    ::dds::domain::qos::DomainParticipantQos pqos = participant.default_participant_qos();

    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_NO_THROW(participant.default_participant_qos(PARTICIPANT_QOS_DEFAULT));
}

TEST(ParticipantTests, ChangeDomainParticipantQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipantQos qos;
    participant->get_qos(qos);

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_TRUE(participant->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    DomainParticipantQos pqos;
    participant->get_qos(pqos);

    ASSERT_FALSE(pqos == PARTICIPANT_QOS_DEFAULT);
    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(qos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);

}

TEST(ParticipantTests, ChangePSMDomainParticipantQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::core::null;
    participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::domain::qos::DomainParticipantQos qos = participant.qos();

    ASSERT_EQ(qos, PARTICIPANT_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_NO_THROW(participant.qos(qos));
    ::dds::domain::qos::DomainParticipantQos pqos;
    pqos = participant.qos();

    ASSERT_FALSE(pqos == PARTICIPANT_QOS_DEFAULT);
    ASSERT_EQ(qos, pqos);
    ASSERT_EQ(qos.entity_factory().autoenable_created_entities, false);

}

TEST(ParticipantTests, EntityFactoryBehavior)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    {
        DomainParticipantFactoryQos qos;
        qos.entity_factory().autoenable_created_entities = false;

        ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    }

    // Ensure that participant is created disabled.
    DomainParticipant* participant = factory->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    // Participant is disabled. This means we can change an inmutable qos.
    DomainParticipantQos qos = PARTICIPANT_QOS_DEFAULT;
    qos.wire_protocol().builtin.avoid_builtin_multicast = !qos.wire_protocol().builtin.avoid_builtin_multicast;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->set_qos(qos));

    // Creating lower entities should create them disabled
    Publisher* pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, pub);
    EXPECT_FALSE(pub->is_enabled());

    Subscriber* sub = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, sub);
    EXPECT_FALSE(sub->is_enabled());

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    EXPECT_FALSE(topic->is_enabled());

    // Enabling should fail on lower entities until participant is enabled
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, pub->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, sub->enable());

    // Enable participant and check idempotency of enable
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());

    // As the participant was created with the default value for ENTITY_FACTORY,
    // lower entities should have been automatically enabled.
    EXPECT_TRUE(pub->is_enabled());
    EXPECT_TRUE(sub->is_enabled());

    // Now that participant is enabled, we should not be able change an inmutable qos.
    ASSERT_EQ(ReturnCode_t::RETCODE_IMMUTABLE_POLICY, participant->set_qos(PARTICIPANT_QOS_DEFAULT));

    // Created entities should now be automatically enabled
    Subscriber* sub2 = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, sub2);
    EXPECT_TRUE(sub2->is_enabled());

    // We can change ENTITY_FACTORY on the participant
    qos.entity_factory().autoenable_created_entities = false;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->set_qos(qos));

    // Lower entities should now be created disabled
    Publisher* pub2 = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, pub2);
    EXPECT_FALSE(pub2->is_enabled());

    // But could be enabled afterwards
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, pub2->enable());
    EXPECT_TRUE(pub2->is_enabled());

    // Check idempotency of enable on entities
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, pub->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, pub2->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, sub->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, sub2->enable());

    // Delete lower entities
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(sub2));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(pub2));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(sub));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(pub));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_topic(topic));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant));
}

TEST(ParticipantTests, CreatePublisher)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    ASSERT_NE(publisher, nullptr);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

void check_publisher_with_profile (
        Publisher* publisher,
        const std::string& profile_name)
{
    PublisherQos qos;
    publisher->get_qos(qos);

    PublisherAttributes publisher_atts;
    XMLProfileManager::fillPublisherAttributes(profile_name, publisher_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.group_data().dataVec() == publisher_atts.qos.m_groupData.dataVec());
    ASSERT_TRUE(qos.partition() == publisher_atts.qos.m_partition);
    ASSERT_TRUE(qos.presentation() == publisher_atts.qos.m_presentation);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.entity_factory() == PUBLISHER_QOS_DEFAULT.entity_factory());
}

TEST(ParticipantTests, CreatePublisherWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    //publisher using the default profile
    Publisher* default_publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(default_publisher, nullptr);
    check_publisher_with_profile(default_publisher, "test_default_publisher_profile");
    ASSERT_TRUE(participant->delete_publisher(default_publisher) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    Publisher* publisher = participant->create_publisher_with_profile("test_publisher_profile");
    ASSERT_NE(publisher, nullptr);
    check_publisher_with_profile(publisher, "test_publisher_profile");
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreatePSMPublisher)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::core::null;
    publisher = ::dds::pub::Publisher(participant);

    ASSERT_NE(publisher, ::dds::core::null);
}

TEST(ParticipantTests, ChangeDefaultPublisherQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_TRUE(participant->set_default_publisher_qos(PUBLISHER_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);

    PublisherQos qos;
    ASSERT_TRUE(participant->get_default_publisher_qos(qos) == ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_TRUE(participant->set_default_publisher_qos(qos) == ReturnCode_t::RETCODE_OK);

    PublisherQos pqos;
    ASSERT_TRUE(participant->get_default_publisher_qos(pqos) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultPublisherQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::qos::PublisherQos qos = participant.default_publisher_qos();
    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_NO_THROW(participant.default_publisher_qos(qos));

    ::dds::pub::qos::PublisherQos pqos = participant.default_publisher_qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
}

TEST(ParticipantTests, CreateSubscriber)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

void check_subscriber_with_profile (
        Subscriber* subscriber,
        const std::string& profile_name)
{
    SubscriberQos qos;
    subscriber->get_qos(qos);

    SubscriberAttributes subscriber_atts;
    XMLProfileManager::fillSubscriberAttributes(profile_name, subscriber_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.group_data().dataVec() == subscriber_atts.qos.m_groupData.dataVec());
    ASSERT_TRUE(qos.partition() == subscriber_atts.qos.m_partition);
    ASSERT_TRUE(qos.presentation() == subscriber_atts.qos.m_presentation);

    //Values not implemented on attributes (taken from default QoS)
    ASSERT_TRUE(qos.entity_factory() == SUBSCRIBER_QOS_DEFAULT.entity_factory());
}

TEST(ParticipantTests, GetSubscriberProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Extract qos from profile
    SubscriberQos qos;
    EXPECT_EQ(
        participant->get_subscriber_qos_from_profile("test_subscriber_profile", qos),
        ReturnCode_t::RETCODE_OK);

    Subscriber* subscriber = participant->create_subscriber(qos);
    ASSERT_NE(subscriber, nullptr);

    check_subscriber_with_profile(subscriber, "test_subscriber_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        participant->get_subscriber_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreateSubscriberWithProfile)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    //subscriber using the default profile
    Subscriber* default_subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(default_subscriber, nullptr);
    check_subscriber_with_profile(default_subscriber, "test_default_subscriber_profile");
    ASSERT_TRUE(participant->delete_subscriber(default_subscriber) == ReturnCode_t::RETCODE_OK);

    //participant using non-default profile
    Subscriber* subscriber = participant->create_subscriber_with_profile("test_subscriber_profile");
    ASSERT_NE(subscriber, nullptr);
    check_subscriber_with_profile(subscriber, "test_subscriber_profile");
    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetPublisherProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    // Extract qos from profile
    PublisherQos qos;
    EXPECT_EQ(
        participant->get_publisher_qos_from_profile("test_publisher_profile", qos),
        ReturnCode_t::RETCODE_OK);

    Publisher* publisher = participant->create_publisher(qos);
    ASSERT_NE(publisher, nullptr);

    check_publisher_with_profile(publisher, "test_publisher_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        participant->get_publisher_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


TEST(ParticipantTests, CreatePSMSubscriber)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::Subscriber subscriber = ::dds::core::null;
    subscriber = ::dds::sub::Subscriber(participant, SUBSCRIBER_QOS_DEFAULT);

    ASSERT_NE(subscriber, ::dds::core::null);
}

TEST(ParticipantTests, DeletePublisher)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteSubscriber)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    ASSERT_TRUE(participant->delete_subscriber(subscriber) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangeDefaultSubscriberQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_EQ(participant->set_default_subscriber_qos(SUBSCRIBER_QOS_DEFAULT), ReturnCode_t::RETCODE_OK);

    SubscriberQos qos;
    ASSERT_EQ(participant->get_default_subscriber_qos(qos), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_EQ(participant->set_default_subscriber_qos(qos), ReturnCode_t::RETCODE_OK);

    SubscriberQos pqos;
    ASSERT_EQ(participant->get_default_subscriber_qos(pqos), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(pqos == qos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultSubscriberQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::sub::qos::SubscriberQos qos = participant.default_subscriber_qos();
    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    ASSERT_NO_THROW(participant.default_subscriber_qos(qos));

    ::dds::sub::qos::SubscriberQos pqos = participant.default_subscriber_qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
}

TEST(ParticipantTests, ChangeDefaultTopicQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_TRUE(participant->set_default_topic_qos(TOPIC_QOS_DEFAULT) == ReturnCode_t::RETCODE_OK);

    TopicQos qos;
    participant->get_default_topic_qos(qos);

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    ASSERT_TRUE(participant->set_default_topic_qos(qos) == ReturnCode_t::RETCODE_OK);

    TopicQos tqos;
    participant->get_default_topic_qos(tqos);

    ASSERT_EQ(qos, tqos);
    ASSERT_EQ(tqos.reliability().kind, BEST_EFFORT_RELIABILITY_QOS);

    qos.durability().kind = PERSISTENT_DURABILITY_QOS;
    ASSERT_FALSE(participant->set_default_topic_qos(qos) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ChangePSMDefaultTopicQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::topic::qos::TopicQos qos = participant.default_topic_qos();

    ASSERT_EQ(qos, TOPIC_QOS_DEFAULT);

    qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;

    ASSERT_NO_THROW(participant.default_topic_qos(qos));

    ::dds::topic::qos::TopicQos tqos = participant.default_topic_qos();
    ASSERT_EQ(qos, tqos);
    ASSERT_EQ(tqos.ownership().kind, EXCLUSIVE_OWNERSHIP_QOS);
}

void check_topic_with_profile (
        Topic* topic,
        const std::string& profile_name)
{
    TopicQos qos;
    topic->get_qos(qos);

    TopicAttributesQos topic_atts;
    XMLProfileManager::fillTopicAttributes(profile_name, topic_atts);

    //Values taken from profile
    ASSERT_TRUE(qos.history() == topic_atts.historyQos);
    ASSERT_TRUE(qos.resource_limits() == topic_atts.resourceLimitsQos);
}

TEST(ParticipantTests, GetTopicProfileQos)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // Extract qos from profile
    TopicQos qos;
    EXPECT_EQ(
        participant->get_topic_qos_from_profile("test_topic_profile", qos),
        ReturnCode_t::RETCODE_OK);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), qos);
    ASSERT_NE(topic, nullptr);


    check_topic_with_profile(topic, "test_topic_profile");

    // Test return when a non-existent profile is used
    EXPECT_EQ(
        participant->get_topic_qos_from_profile("incorrect_profile_name", qos),
        ReturnCode_t::RETCODE_BAD_PARAMETER);

    // Clean up
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreateTopic)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");

    // Topic using the default profile
    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);

    // Topic using non-default profile
    Topic* topic_profile = participant->create_topic_with_profile("footopic", "footype", "test_topic_profile");
    ASSERT_NE(topic_profile, nullptr);
    check_topic_with_profile(topic_profile, "test_topic_profile");
    ASSERT_TRUE(participant->delete_topic(topic_profile) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, PSMCreateTopic)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant.delegate().get(), "footype");

    ::dds::topic::Topic topic = ::dds::core::null;
    topic = ::dds::topic::Topic(participant, "footopic", "footype", TOPIC_QOS_DEFAULT);

    ASSERT_NE(topic, ::dds::core::null);
}

TEST(ParticipantTests, DeleteTopic)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipant* participant2 =
            DomainParticipantFactory::get_instance()->create_participant(1, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");

    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    ASSERT_TRUE(participant->delete_topic(nullptr) == ReturnCode_t::RETCODE_BAD_PARAMETER);
    ASSERT_TRUE(participant2->delete_topic(topic) == ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant2) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, LookupTopicDescription)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    const std::string topic_name("footopic");

    // Topic not created yet. Should return nil
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), nullptr);

    // After topic creation ...
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");
    Topic* topic = participant->create_topic(topic_name, "footype", TOPIC_QOS_DEFAULT);
    EXPECT_NE(topic, nullptr);

    // ... the topic should be returned.
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), topic);

    // After topic deletion, should return nil
    EXPECT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->lookup_topicdescription(topic_name), nullptr);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteTopicInUse)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant, "footype");

    Topic* topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);

    topic = participant->create_topic("footopic", "footype", TOPIC_QOS_DEFAULT);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    DataWriter* data_writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(publisher->delete_datawriter(data_writer), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


void set_listener_test (
        DomainParticipant* participant,
        DomainParticipantListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(participant->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->get_status_mask(), mask);
}

class CustomListener : public DomainParticipantListener
{

};

TEST(ParticipantTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT, &listener);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(participant->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<DomainParticipant*, DomainParticipantListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { participant, &listener, StatusMask::liveliness_lost() },
        { participant, &listener, StatusMask::offered_deadline_missed() },
        { participant, &listener, StatusMask::offered_incompatible_qos() },
        { participant, &listener, StatusMask::publication_matched() },
        { participant, &listener, StatusMask::data_on_readers() },
        { participant, &listener, StatusMask::data_available() },
        { participant, &listener, StatusMask::sample_rejected() },
        { participant, &listener, StatusMask::liveliness_changed() },
        { participant, &listener, StatusMask::requested_deadline_missed() },
        { participant, &listener, StatusMask::requested_incompatible_qos() },
        { participant, &listener, StatusMask::subscription_matched() },
        { participant, &listener, StatusMask::sample_lost() },
        //all except one
        { participant, &listener, StatusMask::all() >> StatusMask::liveliness_lost() },
        { participant, &listener, StatusMask::all() >> StatusMask::offered_deadline_missed() },
        { participant, &listener, StatusMask::all() >> StatusMask::offered_incompatible_qos() },
        { participant, &listener, StatusMask::all() >> StatusMask::publication_matched() },
        { participant, &listener, StatusMask::all() >> StatusMask::data_on_readers() },
        { participant, &listener, StatusMask::all() >> StatusMask::data_available() },
        { participant, &listener, StatusMask::all() >> StatusMask::sample_rejected() },
        { participant, &listener, StatusMask::all() >> StatusMask::liveliness_changed() },
        { participant, &listener, StatusMask::all() >> StatusMask::requested_deadline_missed() },
        { participant, &listener, StatusMask::all() >> StatusMask::requested_incompatible_qos() },
        { participant, &listener, StatusMask::all() >> StatusMask::subscription_matched() },
        { participant, &listener, StatusMask::all() >> StatusMask::sample_lost() },
        //all and none
        { participant, &listener, StatusMask::all() },
        { participant, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CheckDomainParticipantQos)
{

    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    {
        DomainParticipantFactoryQos qos;
        qos.entity_factory().autoenable_created_entities = false;

        ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    }

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipantQos qos;
    ASSERT_TRUE(participant->get_qos(qos) == ReturnCode_t::RETCODE_OK);

    qos.user_data().set_max_size(5);
    std::vector<eprosima::fastrtps::rtps::octet> my_data {0, 1, 2, 3, 4};
    qos.user_data().setValue(my_data);
    ASSERT_TRUE(participant->set_qos(qos) == ReturnCode_t::RETCODE_OK);

    qos.allocation().data_limits.max_user_data = 1;
    ASSERT_EQ(qos.allocation().data_limits.max_user_data, 1);
    ASSERT_TRUE(participant->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, DeleteEntitiesNegativeClauses)
{
    DomainParticipant* participant_1 =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    DomainParticipant* participant_2 =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    Subscriber* subscriber_1 = participant_1->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber_1, nullptr);
    ASSERT_EQ(participant_2->delete_subscriber(subscriber_1), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant_1->delete_subscriber(subscriber_1), ReturnCode_t::RETCODE_OK);

    Publisher* publisher_1 = participant_1->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher_1, nullptr);
    ASSERT_EQ(participant_2->delete_publisher(publisher_1), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(participant_1->delete_publisher(publisher_1), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant_1) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant_2) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreateEntitiesWithProfileNegativeClauses)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    // Create publisher with an empty profile should return nullptr
    Publisher* publisher = participant->create_publisher_with_profile("");
    ASSERT_EQ(publisher, nullptr);

    // Create subscriber with an empty profile should return nullptr
    Subscriber* subscriber = participant->create_subscriber_with_profile("");
    ASSERT_EQ(subscriber, nullptr);

    // Create topic with an empty profile should return nullptr
    Topic* topic = participant->create_topic_with_profile("footopic", "footype", "");
    ASSERT_EQ(topic, nullptr);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, RegisterTypeNegativeClauses)
{
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("test_xml_profiles.xml");
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TopicDataTypeMock* data_type = new TopicDataTypeMock();
    data_type->clearName();

    TypeSupport type(data_type);
    EXPECT_EQ(type.register_type(participant), ReturnCode_t::RETCODE_BAD_PARAMETER);

    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, AssertLivelinesNegativeClauses)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    {
        DomainParticipantFactoryQos qos;
        qos.entity_factory().autoenable_created_entities = false;

        ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    }

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    ASSERT_EQ(participant->assert_liveliness(), ReturnCode_t::RETCODE_NOT_ENABLED);

    DomainParticipantQos pqos;
    ASSERT_EQ(participant->get_qos(pqos), ReturnCode_t::RETCODE_OK);
    pqos.wire_protocol().builtin.use_WriterLivelinessProtocol = false;
    ASSERT_EQ(participant->set_qos(pqos), ReturnCode_t::RETCODE_OK);

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());
    ASSERT_EQ(participant->assert_liveliness(), ReturnCode_t::RETCODE_ERROR);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetCurrentTime)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    eprosima::fastrtps::Time_t now;
    ASSERT_EQ(participant->get_current_time(now), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetParticipantConst)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    const DomainParticipant* participant_pub = publisher->get_participant();

    ASSERT_EQ(participant_pub->guid(), participant->guid());

    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, GetParticipantNames)
{
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();

    {
        DomainParticipantFactoryQos qos;
        qos.entity_factory().autoenable_created_entities = false;

        ASSERT_TRUE(factory->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    }

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);
    ASSERT_FALSE(participant->is_enabled());

    std::vector<std::string> participant_names = participant->get_participant_names();
    ASSERT_TRUE(participant_names.empty());

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->enable());
    EXPECT_TRUE(participant->is_enabled());
    participant_names = participant->get_participant_names();
    ASSERT_FALSE(participant_names.empty());

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, CreateTopicNegativeClauses)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic;
    topic = participant->create_topic("footopic", "fake_type_name", TOPIC_QOS_DEFAULT);
    ASSERT_EQ(topic, nullptr);

    TopicQos tqos;
    participant->get_default_topic_qos(tqos);
    tqos.durability().kind = PERSISTENT_DURABILITY_QOS;
    topic = participant->create_topic("footopic", type.get_type_name(), tqos);
    ASSERT_EQ(topic, nullptr);

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(ParticipantTests, ContainsEntity)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t topic_ihandle = topic->get_instance_handle();
    ASSERT_TRUE(participant->contains_entity(topic_ihandle, false));

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t pub_ihandle = publisher->get_instance_handle();
    ASSERT_TRUE(participant->contains_entity(pub_ihandle, false));

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t sub_ihandle = subscriber->get_instance_handle();
    ASSERT_TRUE(participant->contains_entity(sub_ihandle, false));

    DataWriter* data_writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(data_writer, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t data_writer_ihandle = data_writer->get_instance_handle();
    ASSERT_TRUE(participant->contains_entity(data_writer_ihandle, true));

    DataReader* data_reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(data_reader, nullptr);
    eprosima::fastrtps::rtps::InstanceHandle_t data_reader_ihandle = data_reader->get_instance_handle();
    ASSERT_TRUE(participant->contains_entity(data_reader_ihandle, true));

    ASSERT_EQ(publisher->delete_datawriter(data_writer), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(subscriber->delete_datareader(data_reader), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_subscriber(subscriber), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);

    ASSERT_FALSE(participant->contains_entity(pub_ihandle, false));

    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}


} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
