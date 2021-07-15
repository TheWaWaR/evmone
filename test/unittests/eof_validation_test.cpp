// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2021 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include <evmone/eof.hpp>
#include <gtest/gtest.h>
#include <test/utils/utils.hpp>

using namespace evmone;

namespace
{
inline EOFValidationErrror validate_eof(evmc_revision rev, bytes_view code) noexcept
{
    return ::validate_eof(rev, code.data(), code.size());
}
}  // namespace

TEST(eof_validation, validate_empty_code)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, {}), EOFValidationErrror::invalid_prefix);
}

TEST(eof_validation, validate_EOF_prefix)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("00")), EOFValidationErrror::invalid_prefix);
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("FE")), EOFValidationErrror::invalid_prefix);
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EF")), EOFValidationErrror::invalid_prefix);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCA")), EOFValidationErrror::invalid_prefix);
    EXPECT_EQ(
        validate_eof(EVMC_SHANGHAI, from_hex("EFCBFE01")), EOFValidationErrror::invalid_prefix);
    EXPECT_EQ(
        validate_eof(EVMC_SHANGHAI, from_hex("EFCAFF01")), EOFValidationErrror::invalid_prefix);

    EXPECT_EQ(
        validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE")), EOFValidationErrror::eof_version_unknown);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01")),
        EOFValidationErrror::section_headers_not_terminated);
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE02")),
        EOFValidationErrror::section_headers_not_terminated);
}

// TODO tests from pre-Shanghai

TEST(eof_validation, validate_EOF_version)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE03")),
        EOFValidationErrror::eof_version_unknown);
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFEFF")),
        EOFValidationErrror::eof_version_unknown);
}

TEST(eof_validation, minimal_valid_EOF1_code)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 00 FE")),
        EOFValidationErrror::success);
}

TEST(eof_validation, minimal_valid_EOF1_code_with_data)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 020001 00 FE DA")),
        EOFValidationErrror::success);
}

TEST(eof_validation, EOF1_code_section_missing)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 00")),
        EOFValidationErrror::code_section_missing);
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 020001 DA")),
        EOFValidationErrror::code_section_missing);
}

TEST(eof_validation, EOF1_multiple_code_sections)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 010001 00 FE FE")),
        EOFValidationErrror::multiple_code_sections);
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 010001 020001 00 FE FE DA")),
        EOFValidationErrror::multiple_code_sections);
}

TEST(eof_validation, EOF1_multiple_data_sections)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 020001 020001 00 FE DA DA")),
        EOFValidationErrror::multiple_data_sections);
}

TEST(eof_validation, EOF1_table_section)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 030002 00 FE 0001")),
        EOFValidationErrror::unknown_section_id);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE01 010001 020001 030002 00 FE DA 0001")),
        EOFValidationErrror::unknown_section_id);
}

TEST(eof_validation, minimal_valid_EOF2)
{
    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE02 010001 00 FE")),
        EOFValidationErrror::success);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE02 010001 020001 00 FE DA")),
        EOFValidationErrror::success);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE02 010001 030002 00 FE 0001")),
        EOFValidationErrror::success);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE02 010001 020001 030002 00 FE DA 0001")),
        EOFValidationErrror::success);
}

TEST(eof_validation, multiple_table_sections)
{
    EXPECT_EQ(
        validate_eof(EVMC_SHANGHAI, from_hex("EFCAFE02 010001 030002 030004 00 FE 0001 00010002")),
        EOFValidationErrror::success);

    EXPECT_EQ(validate_eof(EVMC_SHANGHAI,
                  from_hex("EFCAFE02 010001 020001 030002 030004 00 FE DA 0001 00010002")),
        EOFValidationErrror::success);
}
