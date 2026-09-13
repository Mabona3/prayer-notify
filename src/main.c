/*
 * The prayer-notify application.
 *
 * Copyright (C) 2025 Mabona <mazenkhaled2003@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "app.h"

int main(int argc, char *argv[]) {
    AppContext app_context = {0};
    int ret;
    if ((ret = app_init(&app_context, argc, argv)) != 0) {
        return ret != 1;
    }
    app_run(&app_context);
    app_quit(&app_context);
    return 0;
}
