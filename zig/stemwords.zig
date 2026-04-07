const std = @import("std");
const snowball = @import("env.zig");
const alg = @import("algorithms.zig");

fn findStemmer(name: []const u8) ?alg.StemFn {
    for (alg.algorithms) |a| {
        if (std.mem.eql(u8, a.name, name)) return a.stem;
    }
    return null;
}

fn readFile(io: std.Io, allocator: std.mem.Allocator, path: []const u8) ![]u8 {
    const file = try std.Io.Dir.cwd().openFile(io, path, .{});
    defer file.close(io);
    const stat = try file.stat(io);
    const size: usize = @intCast(stat.size);
    const buf = try allocator.alloc(u8, size);
    errdefer allocator.free(buf);
    const n = try file.readPositionalAll(io, buf, 0);
    return buf[0..n];
}

fn writeFile(io: std.Io, path: []const u8, data: []const u8) !void {
    const file = try std.Io.Dir.cwd().createFile(io, path, .{});
    defer file.close(io);
    try file.writePositionalAll(io, data, 0);
}

pub fn main(init: std.process.Init) !void {
    const io = init.io;
    const allocator = init.gpa;

    var args_iter = std.process.Args.Iterator.init(init.minimal.args);
    _ = args_iter.next(); // skip program name

    var language: ?[]const u8 = null;
    var input_path: ?[]const u8 = null;
    var output_path: ?[]const u8 = null;

    while (args_iter.next()) |arg| {
        if (std.mem.eql(u8, arg, "-l")) {
            language = args_iter.next();
        } else if (std.mem.eql(u8, arg, "-i")) {
            input_path = args_iter.next();
        } else if (std.mem.eql(u8, arg, "-o")) {
            output_path = args_iter.next();
        }
    }

    const lang = language orelse std.process.fatal("error: -l language required", .{});
    const stem_fn = findStemmer(lang) orelse
        std.process.fatal("error: unknown language '{s}'", .{lang});

    var env = snowball.Env.init(allocator);
    defer env.deinit();

    if (input_path) |p| {
        // File input: read all, process, write all
        const input_data = try readFile(io, allocator, p);
        defer allocator.free(input_data);

        var output = std.ArrayList(u8).initCapacity(allocator, input_data.len) catch
            std.process.fatal("out of memory", .{});
        defer output.deinit(allocator);

        var rest = input_data;
        while (std.mem.indexOfScalar(u8, rest, '\n')) |nl| {
            const line = rest[0..nl];
            if (line.len > 0) {
                try env.setCurrent(line);
                _ = stem_fn(&env);
                output.appendSlice(allocator, env.getCurrent()) catch
                    std.process.fatal("out of memory", .{});
            }
            output.append(allocator, '\n') catch
                std.process.fatal("out of memory", .{});
            rest = rest[nl + 1 ..];
        }
        if (rest.len > 0) {
            try env.setCurrent(rest);
            _ = stem_fn(&env);
            output.appendSlice(allocator, env.getCurrent()) catch
                std.process.fatal("out of memory", .{});
            output.append(allocator, '\n') catch
                std.process.fatal("out of memory", .{});
        }

        if (output_path) |out_path| {
            try writeFile(io, out_path, output.items);
        } else {
            var write_buf: [8192]u8 = undefined;
            var writer = std.Io.File.stdout().writerStreaming(io, &write_buf);
            try writer.interface.writeAll(output.items);
            try writer.interface.flush();
        }
    } else {
        // Stdin: process line by line, stream output
        var read_buf: [8192]u8 = undefined;
        var reader = std.Io.File.stdin().readerStreaming(io, &read_buf);

        const out_file = if (output_path) |out_path|
            try std.Io.Dir.cwd().createFile(io, out_path, .{})
        else
            null;
        defer if (out_file) |f| f.close(io);

        var write_buf: [8192]u8 = undefined;
        var writer = if (out_file) |f|
            f.writerStreaming(io, &write_buf)
        else
            std.Io.File.stdout().writerStreaming(io, &write_buf);

        while (try reader.interface.takeDelimiter('\n')) |line| {
            if (line.len > 0) {
                try env.setCurrent(line);
                _ = stem_fn(&env);
                try writer.interface.writeAll(env.getCurrent());
            }
            try writer.interface.writeByte('\n');
        }
        try writer.interface.flush();
    }
}
