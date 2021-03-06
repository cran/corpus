context("text_format")


test_that("'format' can handle short text", {
    raw <- c(NA, "", "a", "foo", "short text")
    text <- as_corpus_text(raw)
    names(raw) <- names(text)

    expect_equal(format(text, justify = "none", na.print = "NA"),
                 format(raw, justify = "none"))

    expect_equal(format(text, justify = "left", na.print = "NA"),
                 format(raw, justify = "left"))

    expect_equal(format(text, justify = "centre", na.print = "NA"),
                 format(raw, justify = "centre"))

    expect_equal(format(text, justify = "right", na.print = "NA"),
                 format(raw, justify = "right"))
})


test_that("'format' can handle long text in Unicode locale", {
    raw    <- c(NA, "", "a", "ab", "foo", "food",     "short text",
                "\u6027", "\u6027\u6027", "\u6027?")
    Encoding(raw) <- "UTF-8"

    short  <- c(NA, "", "a", "ab", "fo\u2026", "fo\u2026", "sh\u2026",
                "\u6027", "\u6027\u2026", "\u6027\u2026")
    Encoding(short) <- "UTF-8"

    rshort <- c(NA, "", "a", "ab", "\u2026oo", "\u2026od", "\u2026xt",
                "\u6027", "\u2026\u6027", "\u2026?")
    Encoding(rshort) <- "UTF-8"

    text <- as_corpus_text(raw)
    names(raw) <- names(text)
    names(short) <- names(text)
    names(rshort) <- names(text)

    ctype <- switch_ctype("UTF-8")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))
    skip_on_os("windows") # windows can't format \u6027

    expect_equal(format(text, chars = 2, justify = "none", na.print = "NA"),
                 format(short, justify = "none"))

    expect_equal(format(text, chars = 2, justify = "left", na.print = "NA"),
                 format(short, justify = "left"))

    expect_equal(format(text, chars = 2, justify = "centre", na.print = "NA"),
                 format(short, justify = "centre"))

    expect_equal(format(text, chars = 2, justify = "right", na.print = "NA"),
                 format(rshort, justify = "right"))
})


test_that("'format' can handle long text in UTF-8 locale, part 2", {
    raw    <- c(NA, "", "a", "\n", "ab", "foo", "food",     "short text",
                "\u6027", "\u6027\u6027", "\u6027?")
    short  <- c(NA, "", "a", "\u2026", "a\u2026", "f\u2026", "f\u2026",
                "s\u2026", "\u2026", "\u2026", "\u2026")
    rshort <- c(NA, "", "a", "\u2026", "\u2026b", "\u2026o", "\u2026d",
                "\u2026t", "\u2026", "\u2026", "\u2026?")
    text <- as_corpus_text(raw)
    names(raw) <- names(text)
    names(short) <- names(text)
    names(rshort) <- names(text)

    ctype <- switch_ctype("UTF-8")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))

    expect_equal(format(text, chars = 1, justify = "none", na.encode = FALSE),
                 format(short, justify = "none", na.encode = FALSE))

    expect_equal(format(text, chars = 1, justify = "left", na.encode = FALSE),
                 format(short, justify = "left", na.encode = FALSE))

    expect_equal(format(text, chars = 1, justify = "centre", na.encode = FALSE),
                 format(short, justify = "centre", na.encode = FALSE))

    expect_equal(format(text, chars = 1, justify = "right", na.encode = FALSE),
                 format(rshort, justify = "right", na.encode = FALSE))
})


test_that("'format' can handle high code points in C locale", {
    ctype <- switch_ctype("C")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))

    raw <- c(intToUtf8(0x00010000), intToUtf8(0x0010ffff))
    text <- as_corpus_text(raw)
    fmt <- raw

    expect_equal(format(text, justify = "left"), fmt)
    expect_equal(format(text, justify = "right"), fmt)
})


test_that("'format' can handle high code points in Unicode locale", {
    ctype <- switch_ctype("UTF-8")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))
    skip_on_os("windows") # no Unicode above 0xFFFF on Windows

    raw   <- c(intToUtf8(0x00010000), intToUtf8(0x010ffff))
    left  <- c(paste0(intToUtf8(0x00010000), "         "), intToUtf8(0x010ffff))
    right <- c(paste0("         ", intToUtf8(0x00010000)), intToUtf8(0x010ffff))
    text <- as_corpus_text(raw)

    expect_equal(format(text, justify = "left"), left)
    expect_equal(format(text, justify = "right"), right)
})


test_that("'format' can handle ignorable code points", {
    raw <- "\u200B"
    text <- as_corpus_text(raw)

    ctype <- switch_ctype("C")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))

    expect_equal(format(text, justify = "left"), raw)
    expect_equal(format(text, justify = "centre"), raw)
    expect_equal(format(text, justify = "right"), raw)

    switch_ctype("UTF-8")

    expect_equal(format(text, justify = "left"), raw)
    expect_equal(format(text, justify = "centre"), raw)
    expect_equal(format(text, justify = "right"), raw)
})


test_that("'format' can handle marks", {
    raw <- "\u1e0d\u0307"
    text <- as_corpus_text(raw)

    ctype <- switch_ctype("C")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))
    
    skip_on_os("windows")
    expect_equal(format(text, chars = 6, justify = "left"), "...")
    
    skip_on_os("windows")
    expect_equal(format(text, chars = 6, justify = "centre"), "...")
    skip_on_os("windows")
    expect_equal(format(text, chars = 5, justify = "right"), "...")

    switch_ctype("UTF-8")
    
    skip_on_os("windows")
    expect_equal(format(text, chars = 1, justify = "left"), raw)
    
    skip_on_os("windows")
    expect_equal(format(text, chars = 1, justify = "centre"), raw)
    
    skip_on_os("windows")
    expect_equal(format(text, chars = 1, justify = "right"), raw)
})


test_that("'format' can handle UTF-8 'Other' codes", {
    raw <- "\u2072" # unassigned
    text <- as_corpus_text(raw)

    ctype <- switch_ctype("C")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))

    expect_equal(format(text, justify = "left"), raw)
    expect_equal(format(text, justify = "centre"), raw)
    expect_equal(format(text, justify = "right"), raw)

    switch_ctype("UTF-8")

    expect_equal(format(text, justify = "left"), raw)
    expect_equal(format(text, justify = "centre"), raw)
    expect_equal(format(text, justify = "right"), raw)
})


test_that("'format' can handle zero, or NULL chars", {
    text <- as_corpus_text("foo")

    ctype <- switch_ctype("C")
    on.exit(Sys.setlocale("LC_CTYPE", ctype))
    skip_on_os("windows")
    
    expect_equal(as.character(format(text, chars = 0, justify = "left")),
                 "...")
    skip_on_os("windows")
    
    expect_equal(as.character(format(text, chars = 0, justify = "centre")),
                 "...")
    skip_on_os("windows")
    
    expect_equal(as.character(format(text, chars = 0, justify = "right")),
                 "...")
    skip_on_os("windows")
    
    expect_equal(as.character(format(text, chars = NULL, justify = "left")),
                 "foo")
    skip_on_os("windows")
    
    expect_equal(as.character(format(text, chars = NULL, justify = "centre")),
                 "foo")
    skip_on_os("windows")
    
    expect_equal(as.character(format(text, chars = NULL, justify = "right")),
                 "foo")
})


test_that("'format' can skip NA", {
    text <- as_corpus_text(NA)
    expect_equal(as.character(format(text, na.encode = FALSE)), NA_character_)
})


test_that("'format' can set minimum width", {
    raw <- c("a", "ab", "abc")
    text <- as_corpus_text(raw)
    names(raw) <- names(text)

    expect_equal(format(text, justify = "none", width = 5),
                 format(raw, justify = "none", width = 5))
    expect_equal(format(text, justify = "left", width = 5),
                 format(raw, justify = "left", width = 5))
    expect_equal(format(text, justify = "centre", width = 5),
                 format(raw, justify = "centre", width = 5))
    expect_equal(format(text, justify = "right", width = 5),
                 format(raw, justify = "right", width = 5))
})


test_that("'format' error for invalid justify", {
    text <- as_corpus_text("")
    expect_error(format(text, justify = "wild"),
                 paste("'justify' must be one of the following:",
                       paste(dQuote(c("left", "right", "centre", "none")),
                             collapse = ", ")),
                 fixed = TRUE)
})


test_that("'format' error for invalid logicals", {
    text <- as_corpus_text("")
    expect_error(format(text, na.encode = NA),
                 "'na.encode' must be TRUE or FALSE", fixed = TRUE)
})


test_that("'format' errors for invalid integers", {
    text <- as_corpus_text("")
    expect_error(format(text, chars = "3"),
                 "'chars' must be integer-valued",
                 fixed = TRUE)
    expect_error(format(text, width = "3"),
                 "'width' must be integer-valued",
                 fixed = TRUE)
})
