//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
#include <boost/beast/http/serializer.hpp>

#include <boost/beast/http/string_body.hpp>
#include <boost/beast/unit_test/suite.hpp>

namespace boost {
namespace beast {
namespace http {

class serializer_test : public beast::unit_test::suite
{
public:
    struct deprecated_body
    {
        using value_type = std::string;

        class writer
        {
        public:
            using const_buffers_type =
                boost::asio::const_buffer;

            value_type const& body_;

            template<bool isRequest, class Fields>
            explicit
            writer(message<isRequest, deprecated_body, Fields> const& m):
                body_{m.body()}
            {
            }

            void init(error_code& ec)
            {
                ec.assign(0,  ec.category());
            }

            boost::optional<std::pair<const_buffers_type, bool>>
            get(error_code& ec)
            {
                ec.assign(0, ec.category());
                return {{const_buffers_type{
                    body_.data(), body_.size()}, false}};
            }
        };
    };

    struct const_body
    {
        struct value_type{};

        struct writer
        {
            using const_buffers_type =
                boost::asio::const_buffer;

            template<bool isRequest, class Fields>
            writer(header<isRequest, Fields> const&, value_type const&);

            void
            init(error_code& ec);

            boost::optional<std::pair<const_buffers_type, bool>>
            get(error_code&);
        };
    };

    struct mutable_body
    {
        struct value_type{};

        struct writer
        {
            using const_buffers_type =
                boost::asio::const_buffer;

            template<bool isRequest, class Fields>
            writer(header<isRequest, Fields>&, value_type&);

            void
            init(error_code& ec);

            boost::optional<std::pair<const_buffers_type, bool>>
            get(error_code&);
        };
    };

    BOOST_STATIC_ASSERT(std::is_const<  serializer<
        true, const_body>::value_type>::value);

    BOOST_STATIC_ASSERT(! std::is_const<serializer<
        true, mutable_body>::value_type>::value);

    BOOST_STATIC_ASSERT(std::is_constructible<
        serializer<true, const_body>,
        message   <true, const_body>&>::value);

    BOOST_STATIC_ASSERT(std::is_constructible<
        serializer<true, const_body>,
        message   <true, const_body> const&>::value);

    BOOST_STATIC_ASSERT(std::is_constructible<
        serializer<true, mutable_body>,
        message   <true, mutable_body>&>::value);

    BOOST_STATIC_ASSERT(! std::is_constructible<
        serializer<true, mutable_body>,
        message   <true, mutable_body> const&>::value);

    struct lambda
    {
        std::size_t size;

        template<class ConstBufferSequence>
        void
        operator()(error_code&,
            ConstBufferSequence const& buffers)
        {
            size = boost::asio::buffer_size(buffers);
        }
    };

    void
    testWriteLimit()
    {
        auto const limit = 30;
        lambda visit;
        error_code ec;
        response<string_body> res;
        res.body().append(1000, '*');
        serializer<false, string_body> sr{res};
        sr.limit(limit);
        for(;;)
        {
            sr.next(ec, visit);
            BEAST_EXPECT(visit.size <= limit);
            sr.consume(visit.size);
            if(sr.is_done())
                break;
        }
    }

    void testBodyWriterCtor()
    {
        response<deprecated_body> res;
        request<deprecated_body> req;
        serializer<false, deprecated_body> sr1{res};
        serializer<true, deprecated_body> sr2{req};
        boost::ignore_unused(sr1, sr2);
    }

    void
    run() override
    {
        testWriteLimit();
        testBodyWriterCtor();
    }
};

BEAST_DEFINE_TESTSUITE(beast,http,serializer);

} // http
} // beast
} // boost
