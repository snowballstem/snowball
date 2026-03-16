const std = @import("std");

pub const MaxInt = std.math.maxInt(i32);
pub const MinInt = std.math.minInt(i32);

pub const Among = struct {
    s: []const u8,
    substring_i: i32,
    result: i32,
    method: ?*const fn (*Env, *anyopaque) bool,
};

pub const Env = struct {
    current: []u8,
    cursor: usize,
    limit: usize,
    limit_backward: usize,
    bra: usize,
    ket: usize,
    allocator: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) Env {
        return .{
            .current = &.{},
            .cursor = 0,
            .limit = 0,
            .limit_backward = 0,
            .bra = 0,
            .ket = 0,
            .allocator = allocator,
        };
    }

    pub fn deinit(self: *Env) void {
        if (self.current.len > 0) {
            self.allocator.free(self.current);
        }
    }

    pub fn setCurrent(self: *Env, s: []const u8) !void {
        if (self.current.len > 0) {
            self.allocator.free(self.current);
        }
        self.current = try self.allocator.alloc(u8, s.len);
        @memcpy(self.current, s);
        self.cursor = 0;
        self.limit = s.len;
        self.limit_backward = 0;
        self.bra = 0;
        self.ket = s.len;
    }

    pub fn getCurrent(self: *const Env) []const u8 {
        return self.current[0..self.limit];
    }

    pub fn replaceS(self: *Env, bra_arg: usize, ket_arg: usize, s: []const u8) !i32 {
        const adjustment: i32 = @as(i32, @intCast(s.len)) - (@as(i32, @intCast(ket_arg)) - @as(i32, @intCast(bra_arg)));

        // Always build a new buffer: current[0..bra] + s + current[rsplit..limit]
        // This matches Go's string concatenation semantics and avoids aliasing.
        const rsplit = if (ket_arg < bra_arg) bra_arg else ket_arg;
        const tail_len = self.limit - rsplit;
        const new_len = bra_arg + s.len + tail_len;

        const new_buf = try self.allocator.alloc(u8, new_len);
        @memcpy(new_buf[0..bra_arg], self.current[0..bra_arg]);
        @memcpy(new_buf[bra_arg..][0..s.len], s);
        @memcpy(new_buf[bra_arg + s.len ..][0..tail_len], self.current[rsplit..][0..tail_len]);
        self.allocator.free(self.current);
        self.current = new_buf;

        const new_limit: i32 = @as(i32, @intCast(self.limit)) + adjustment;
        self.limit = @intCast(new_limit);

        if (self.cursor >= ket_arg) {
            const new_cursor: i32 = @as(i32, @intCast(self.cursor)) + adjustment;
            self.cursor = @intCast(new_cursor);
        } else if (self.cursor > bra_arg) {
            self.cursor = bra_arg;
        }

        return adjustment;
    }

    pub fn eqS(self: *Env, s: []const u8) bool {
        if (self.cursor >= self.limit) return false;
        if (self.cursor + s.len > self.limit) return false;
        if (!std.mem.eql(u8, self.current[self.cursor..][0..s.len], s)) return false;
        self.cursor += s.len;
        return true;
    }

    pub fn eqSB(self: *Env, s: []const u8) bool {
        if (@as(i32, @intCast(self.cursor)) - @as(i32, @intCast(self.limit_backward)) < @as(i32, @intCast(s.len))) return false;
        if (!std.mem.eql(u8, self.current[self.cursor - s.len ..][0..s.len], s)) return false;
        self.cursor -= s.len;
        return true;
    }

    pub fn sliceFrom(self: *Env, s: []const u8) !void {
        const bra_val = self.bra;
        _ = try self.replaceS(bra_val, self.ket, s);
        self.ket = bra_val + s.len;
    }

    pub fn sliceDel(self: *Env) !void {
        try self.sliceFrom("");
    }

    pub fn insert(self: *Env, bra_arg: usize, ket_arg: usize, s: []const u8) !void {
        const adjustment = try self.replaceS(bra_arg, ket_arg, s);
        if (bra_arg <= self.bra) {
            self.bra = @intCast(@as(i32, @intCast(self.bra)) + adjustment);
        }
        if (bra_arg <= self.ket) {
            self.ket = @intCast(@as(i32, @intCast(self.ket)) + adjustment);
        }
    }

    pub fn sliceTo(self: *const Env) []const u8 {
        return self.current[self.bra..self.ket];
    }

    pub fn assignTo(self: *const Env) []const u8 {
        return self.getCurrent();
    }

    pub fn nextChar(self: *Env) void {
        self.cursor += 1;
        while (self.cursor < self.current.len and !onCharBoundary(self.current, self.cursor)) {
            self.cursor += 1;
        }
    }

    pub fn prevChar(self: *Env) void {
        self.cursor -= 1;
        while (self.cursor > 0 and !onCharBoundary(self.current, self.cursor)) {
            self.cursor -= 1;
        }
    }

    pub fn hop(self: *Env, delta: i32) bool {
        var d = delta;
        var res = self.cursor;
        while (d > 0) {
            d -= 1;
            if (res >= self.limit) return false;
            res += 1;
            while (res < self.limit and !onCharBoundary(self.current, res)) {
                res += 1;
            }
        }
        self.cursor = res;
        return true;
    }

    pub fn hopChecked(self: *Env, delta: i32) bool {
        return delta >= 0 and self.hop(delta);
    }

    pub fn hopBack(self: *Env, delta: i32) bool {
        var d = delta;
        var res = self.cursor;
        while (d > 0) {
            d -= 1;
            if (res <= self.limit_backward) return false;
            res -= 1;
            while (res > self.limit_backward and !onCharBoundary(self.current, res)) {
                res -= 1;
            }
        }
        self.cursor = res;
        return true;
    }

    pub fn hopBackChecked(self: *Env, delta: i32) bool {
        return delta >= 0 and self.hopBack(delta);
    }

    pub fn inGrouping(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        if (self.cursor >= self.limit) return false;
        const r = decodeRune(self.current[self.cursor..]) orelse return false;
        if (r > max_ch or r < min_ch) return false;
        const idx = r - min_ch;
        if (!inBitmap(chars, idx)) return false;
        self.nextChar();
        return true;
    }

    pub fn inGroupingB(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        if (self.cursor <= self.limit_backward) return false;
        const c = self.cursor;
        self.prevChar();
        const r = decodeRune(self.current[self.cursor..]) orelse {
            self.cursor = c;
            return false;
        };
        if (r > max_ch or r < min_ch) {
            self.cursor = c;
            return false;
        }
        const idx = r - min_ch;
        if (!inBitmap(chars, idx)) {
            self.cursor = c;
            return false;
        }
        return true;
    }

    pub fn outGrouping(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        if (self.cursor >= self.limit) return false;
        const r = decodeRune(self.current[self.cursor..]) orelse return false;
        if (r > max_ch or r < min_ch) {
            self.nextChar();
            return true;
        }
        const idx = r - min_ch;
        if (!inBitmap(chars, idx)) {
            self.nextChar();
            return true;
        }
        return false;
    }

    pub fn outGroupingB(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        if (self.cursor <= self.limit_backward) return false;
        const c = self.cursor;
        self.prevChar();
        const r = decodeRune(self.current[self.cursor..]) orelse {
            self.cursor = c;
            return false;
        };
        if (r > max_ch or r < min_ch) return true;
        const idx = r - min_ch;
        if (!inBitmap(chars, idx)) return true;
        self.cursor = c;
        return false;
    }

    pub fn goInGrouping(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        while (self.cursor < self.limit) {
            const r = decodeRune(self.current[self.cursor..]) orelse return false;
            if (r > max_ch or r < min_ch) return true;
            const idx = r - min_ch;
            if (!inBitmap(chars, idx)) return true;
            self.nextChar();
        }
        return false;
    }

    pub fn goInGroupingB(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        while (self.cursor > self.limit_backward) {
            const c = self.cursor;
            self.prevChar();
            const r = decodeRune(self.current[self.cursor..]) orelse return false;
            if (r > max_ch or r < min_ch) {
                self.cursor = c;
                return true;
            }
            const idx = r - min_ch;
            if (!inBitmap(chars, idx)) {
                self.cursor = c;
                return true;
            }
        }
        return false;
    }

    pub fn goOutGrouping(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        while (self.cursor < self.limit) {
            const r = decodeRune(self.current[self.cursor..]) orelse return false;
            if (r <= max_ch and r >= min_ch) {
                const idx = r - min_ch;
                if (inBitmap(chars, idx)) return true;
            }
            self.nextChar();
        }
        return false;
    }

    pub fn goOutGroupingB(self: *Env, chars: []const u8, min_ch: i32, max_ch: i32) bool {
        while (self.cursor > self.limit_backward) {
            const c = self.cursor;
            self.prevChar();
            const r = decodeRune(self.current[self.cursor..]) orelse return false;
            if (r <= max_ch and r >= min_ch) {
                const idx = r - min_ch;
                if (inBitmap(chars, idx)) {
                    self.cursor = c;
                    return true;
                }
            }
        }
        return false;
    }

    pub fn findAmong(self: *Env, amongs: []const Among, ctx: *anyopaque) i32 {
        var i: i32 = 0;
        var j: i32 = @intCast(amongs.len);

        const c = self.cursor;
        const l = self.limit;

        var common_i: usize = 0;
        var common_j: usize = 0;

        var first_key_inspected = false;
        while (true) {
            const k: usize = @intCast(i + @divTrunc(j - i, 2));
            var diff: i32 = 0;
            var common = @min(common_i, common_j);
            const w = amongs[k];
            var lvar: usize = common;
            while (lvar < w.s.len) : (lvar += 1) {
                if (c + common == l) {
                    diff -= 1;
                    break;
                }
                diff = @as(i32, @intCast(self.current[c + common])) - @as(i32, @intCast(w.s[lvar]));
                if (diff != 0) break;
                common += 1;
            }
            if (diff < 0) {
                j = @intCast(k);
                common_j = common;
            } else {
                i = @intCast(k);
                common_i = common;
            }
            if (j - i <= 1) {
                if (i > 0) break;
                if (j == i) break;
                if (first_key_inspected) break;
                first_key_inspected = true;
            }
        }

        while (true) {
            const w = amongs[@intCast(i)];
            if (common_i >= w.s.len) {
                self.cursor = c + w.s.len;
                if (w.method) |method| {
                    if (method(self, ctx)) {
                        self.cursor = c + w.s.len;
                        return w.result;
                    }
                } else {
                    return w.result;
                }
            }
            i = w.substring_i;
            if (i < 0) return 0;
        }
    }

    pub fn findAmongB(self: *Env, amongs: []const Among, ctx: *anyopaque) i32 {
        var i: i32 = 0;
        var j: i32 = @intCast(amongs.len);

        const c = self.cursor;
        const lb = self.limit_backward;

        var common_i: usize = 0;
        var common_j: usize = 0;

        var first_key_inspected = false;
        while (true) {
            const k: usize = @intCast(i + @divTrunc(j - i, 2));
            var diff: i32 = 0;
            var common = @min(common_i, common_j);
            const w = amongs[k];
            {
                var lvar_signed: i32 = @as(i32, @intCast(w.s.len)) - @as(i32, @intCast(common)) - 1;
                while (lvar_signed >= 0) : (lvar_signed -= 1) {
                    const lvar: usize = @intCast(lvar_signed);
                    if (c - common == lb) {
                        diff -= 1;
                        break;
                    }
                    diff = @as(i32, @intCast(self.current[c - common - 1])) - @as(i32, @intCast(w.s[lvar]));
                    if (diff != 0) break;
                    common += 1;
                }
            }
            if (diff < 0) {
                j = @intCast(k);
                common_j = common;
            } else {
                i = @intCast(k);
                common_i = common;
            }
            if (j - i <= 1) {
                if (i > 0) break;
                if (j == i) break;
                if (first_key_inspected) break;
                first_key_inspected = true;
            }
        }

        while (true) {
            const w = amongs[@intCast(i)];
            if (common_i >= w.s.len) {
                self.cursor = c - w.s.len;
                if (w.method) |method| {
                    if (method(self, ctx)) {
                        self.cursor = c - w.s.len;
                        return w.result;
                    }
                } else {
                    return w.result;
                }
            }
            i = w.substring_i;
            if (i < 0) return 0;
        }
    }

    pub fn clone(self: *const Env) !Env {
        var c = self.*;
        if (self.current.len > 0) {
            c.current = try self.allocator.alloc(u8, self.current.len);
            @memcpy(c.current, self.current);
        }
        return c;
    }

    pub fn copyFrom(self: *Env, other: *const Env) !void {
        if (self.current.len > 0) {
            self.allocator.free(self.current);
        }
        if (other.current.len > 0) {
            self.current = try self.allocator.alloc(u8, other.current.len);
            @memcpy(self.current, other.current);
        } else {
            self.current = &.{};
        }
        self.cursor = other.cursor;
        self.limit = other.limit;
        self.limit_backward = other.limit_backward;
        self.bra = other.bra;
        self.ket = other.ket;
    }

    pub fn debug(self: *const Env, count: i32, line_number: i32) void {
        _ = self;
        std.log.debug("snowball debug, count: {d}, line: {d}", .{ count, line_number });
    }
};

fn inBitmap(chars: []const u8, idx: i32) bool {
    const u_idx: u32 = @intCast(idx);
    return (chars[u_idx >> 3] & (@as(u8, 1) << @intCast(@as(u3, @truncate(u_idx))))) != 0;
}

fn onCharBoundary(s: []const u8, pos: usize) bool {
    if (pos == 0 or pos >= s.len) return true;
    // A byte is a UTF-8 start byte if its top two bits are not 10xxxxxx.
    return (s[pos] & 0xC0) != 0x80;
}

fn decodeRune(s: []const u8) ?i32 {
    if (s.len == 0) return null;
    const b0 = s[0];
    if (b0 < 0x80) return @intCast(b0);
    if (b0 < 0xC0) return null; // continuation byte
    if (b0 < 0xE0) {
        if (s.len < 2) return null;
        return (@as(i32, b0 & 0x1F) << 6) | @as(i32, s[1] & 0x3F);
    }
    if (b0 < 0xF0) {
        if (s.len < 3) return null;
        return (@as(i32, b0 & 0x0F) << 12) | (@as(i32, s[1] & 0x3F) << 6) | @as(i32, s[2] & 0x3F);
    }
    if (s.len < 4) return null;
    return (@as(i32, b0 & 0x07) << 18) | (@as(i32, s[1] & 0x3F) << 12) | (@as(i32, s[2] & 0x3F) << 6) | @as(i32, s[3] & 0x3F);
}

pub fn runeCountInString(s: []const u8) i32 {
    var count: i32 = 0;
    var i: usize = 0;
    while (i < s.len) {
        if (s[i] < 0x80) {
            i += 1;
        } else if (s[i] < 0xE0) {
            i += 2;
        } else if (s[i] < 0xF0) {
            i += 3;
        } else {
            i += 4;
        }
        count += 1;
    }
    return count;
}
