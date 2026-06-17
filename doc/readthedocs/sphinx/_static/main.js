(function ($) {
    function listFilter() { // header is any element, list is an unordered list
        const objectFilter = $("#objectFilter")
        objectFilter
            .change(function () {
                const filter = $(this).val().toUpperCase();
                const listItems = $(".special");
                if (filter === "") {
                    listItems.each(function (idx, li) {
                        $(li).show();
                    });
                } else {
                    listItems.each(function (idx, li) {
                        const thisOne = $(li)[0];
                        if (thisOne.children[0].innerText.toUpperCase().includes(filter)) {
                            $(li).show();
                        } else {
                            $(li).hide();
                        }
                    });
                }
                return false;
            })
            .keyup(function () {
                // fire the above change event after every letter
                $(this).change();
            });
    }

    $(function () {
        listFilter();
    });
}(jQuery));
