# メインライブラリを追加
add_library(parsonFunctions parson.c)

# インストール
install (TARGETS parsonFunctions DESTINATION bin)
install (FILES parson.h DESTINATION include)
