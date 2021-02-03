index_start = 0x102
num_rooms = 55
num_costumes = 25
num_scripts = 160
num_sounds = 70


def lh(lo, hi):
    return (int(hi) << 8) + int(lo)


def get_c64_disk_offset(track, sector):
    sectors_per_track = [21, 19, 18, 17]
    track_count =       [17,  7,  6,  4]

    track = track - 1   # Track number on C64 disk starts at 1

    group = 0
    while track > 0:
        n = min(track, track_count[group])
        sector += n * sectors_per_track[group]
        track -= n
        group += 1

    return 256 * sector


def c64_to_rgb(colour):
    palette = [
        0x00, 0x00, 0x00,	0xFF, 0xFF, 0xFF,	0x7E, 0x35, 0x2B,	0x6E, 0xB7, 0xC1,
        0x7F, 0x3B, 0xA6,	0x5C, 0xA0, 0x35,	0x33, 0x27, 0x99,	0xCB, 0xD7, 0x65,
        0x85, 0x53, 0x1C,	0x50, 0x3C, 0x00,	0xB4, 0x6B, 0x61,	0x4A, 0x4A, 0x4A,
        0x75, 0x75, 0x75,	0xA3, 0xE7, 0x7C,	0x70, 0x64, 0xD6,	0xA3, 0xA3, 0xA3,
        ]
    r = palette[colour*3]
    g = palette[colour*3 + 1]
    b = palette[colour*3 + 2]
    return [r, g, b, r, g, b]


def read_room_disks_and_offsets(disk1, start, count):
    room_disks = []
    for pos in range(start, start+count):
        disk_no = 0
        if disk1[pos] == 0x31:
            disk_no = 1
        elif disk1[pos] == 0x32:
            disk_no = 2
        room_disks.append(disk_no)
    start += count

    room_offsets = []
    for pos in range(start, start+2*count, 2):
        room_offsets.append(get_c64_disk_offset(int(disk1[pos+1]), int(disk1[pos])))
    start += 2*count

    return (room_disks, room_offsets, start)


def read_resource_rooms_and_offsets(disk1, start, count):
    rooms = []
    for pos in range(start, start+count):
        rooms.append(int(disk1[pos]))
    start += count

    offsets = []
    for pos in range(start, start+2*count, 2):
        offsets.append( lh(disk1[pos], disk1[pos+1]) )
    start += 2*count

    return (rooms, offsets, start)


def extract_resource(disk1, disk2, room_disks, room_offsets, resource_room, resource_offset, typestring='', no=0):
    disk_no = room_disks[resource_room]
    if disk_no == 1:
        source = disk1
    elif disk_no == 2:
        source = disk2
    else:
        raise RuntimeError('Disk number for room {} invalid'.format(resource_room))

    start = room_offsets[resource_room] + resource_offset
    length = ( lh(source[start], source[start+1]) )

    if typestring:
        print('From Disk {:d} Room {:02d} at offset {:04x} extracting {:s} {:03d} size {:05d}'.format(disk_no, resource_room, resource_offset, typestring, no, length))

    return source[start:start+length]


def save_resources(disk1, disk2, room_disks, room_offsets, resource_rooms, resource_offsets, typestring):
    for i in range(0,len(resource_rooms)):
        res = extract_resource(disk1, disk2, room_disks, room_offsets, resource_rooms[i], resource_offsets[i], typestring.upper(), i)
        with open('{:s}{:03d}.dat'.format(typestring, i), 'wb') as f:
            f.write(res)


def decodeRLE(room, start, length, loglevel=1, loghead='decodeRLE'):
    lookup = room[start:start+4]
    if loglevel >= 2:
        print('{}: START at {} {:05x}'.format(loghead, start, start))
        print('{}: LOOKUP TABLE: {}'.format(loghead, lookup))

    i = start + 4
    j = 0
    res = [0]*length
    while j < length:
        flags = room[i]
        i += 1
        if (flags & 0x80):
            run_value = lookup[(flags >> 5) & 0x03]
            run_length = (flags & 0x1f) + 1
            if loglevel >= 2:
                print('{}: Indexed run, flags: {:08b}, input: 0x{:05x}, output: {}, length: {}, value: {}'.format(loghead, flags, i-1, j, run_length, run_value))
            while run_length > 0:
                if j >= length and loglevel >= 1:
                    print('{}: warning: overflow in indexed run ({} bytes remaining)'.format(loghead, run_length))
                    return res
                res[j] = run_value
                j += 1
                run_length -= 1
        elif (flags & 0x40):
            run_length = (flags & 0x3f) + 1
            run_value = room[i]
            i += 1
            if loglevel >= 2:
                print('{}: Literal run, flags: {:08b}, input: 0x{:05x}, output: {}, length: {}, value: {}'.format(loghead, flags, i-2, j, run_length, run_value))
            while run_length > 0:
                if j >= length and loglevel >= 1:
                    print('{}: warning: overflow in literal run ({} bytes remaining)'.format(loghead, run_length))
                    return res
                res[j] = run_value
                j += 1
                run_length -= 1
        else:
            run_length = (flags & 0x3f) + 1
            if loglevel >= 2:
                print('{}: Byte stream, flags: {:08b}, input: 0x{:05x}, output: {}, length: {}'.format(loghead, flags, i-1, j, run_length))
            while run_length > 0:
                if j >= length and loglevel >= 1:
                    print('{}: warning: overflow in byte stream ({} bytes remaining)'.format(loghead, run_length))
                    return res
                res[j] = room[i]
                j += 1
                i += 1
                run_length -= 1
    return res


def put_char(image, step, charmap, colours, x, y, c):
    image_i = step*y + 3*x
    char_start = c*8
    for char_row_i in range(8):
        char_row = charmap[char_start + char_row_i]
        image[image_i   :image_i+6]  = c64_to_rgb( colours[ (char_row >> 6) & 0x03 ] )
        image[image_i+6 :image_i+12] = c64_to_rgb( colours[ (char_row >> 4) & 0x03 ] )
        image[image_i+12:image_i+18] = c64_to_rgb( colours[ (char_row >> 2) & 0x03 ] )
        image[image_i+18:image_i+24] = c64_to_rgb( colours[ (char_row >> 0) & 0x03 ] )
        image_i += step


def write_ppm(image, xsize, ysize, filename):
    with open(filename, 'wb') as f:
        f.write(bytearray('P6\n{} {} 255\n'.format(xsize, ysize), 'ascii'))
        f.write(bytearray(image))


def dump_charmap(charmap, colours, room_no):
    colours[3] = colours[3] & 0x0f
    xsize = ysize = 16*9
    step = 3*xsize
    image = [255]*3*xsize*ysize
    for row in range(16):
        for column in range(16):
            c = row*16 + column
            x = column*9
            y = row*9
            put_char(image, step, charmap, colours, x, y, c)

    write_ppm(image, xsize, ysize, 'charmap{:02d}.ppm'.format(room_no))


def dump_room_background(room, room_no):
    width = room[4]
    height = room[5]
    colours = list(room[6:10])
    charmap_start = lh(room[10], room[11])
    picmap_start = lh(room[12], room[13])
    colourmap_start = lh(room[14], room[15])
    charmap = decodeRLE(room, charmap_start, 2048)
    dump_charmap(charmap, colours, room_no)
    picmap = decodeRLE(room, picmap_start, width*height)
    colourmap = decodeRLE(room, colourmap_start, width*height)

    image = [0]*3*width*8*height*8
    step = 3*width*8

    text_i = 0
    for column in range(width):
        for row in range(height):
            c = picmap[text_i]
            colours[3] = colourmap[text_i] & 0x07
            put_char(image, step, charmap, colours, column*8, row*8, c)
            text_i += 1

    write_ppm(image, width*8, height*8, 'room{:02d}.ppm'.format(room_no))

    # dim background before drawing objects
    image = [int(i/4) for i in image]

    num_obj = room[20]
    dummy_offset_i = 28 + 2*num_obj
    dummy_offset = lh(room[dummy_offset_i], room[dummy_offset_i+1])

    for obj_i in range(num_obj-1, -1, -1):
        obj_image_i = 28 + 2*obj_i
        obj_code_i = 28 + 2*num_obj + 2*obj_i
        obj_image_offset = lh(room[obj_image_i], room[obj_image_i+1])
        obj_code_offset = lh(room[obj_code_i], room[obj_code_i+1])
        obj_x = room[obj_code_offset + 6]
        obj_y = room[obj_code_offset + 7] & 0x7f
        obj_width = room[obj_code_offset + 8]
        obj_parent = room[obj_code_offset + 9]
        obj_height = room[obj_code_offset + 12] >> 3

        # for debug
        print(
            '--R{:02d} O{} (x:{} y:{} w:{} h:{}) image:{:05x} code:{:05x} parent:{}'
            .format(room_no, obj_i, obj_x, obj_y, obj_width, obj_height, obj_image_offset, obj_code_offset, obj_parent)
        )

        if obj_image_offset == dummy_offset:
            print('Room {}, object {}: image has dummy offset'.format(room_no, obj_i))
            continue

        if obj_width == 0 or obj_height == 0:
            print('Room {}, object {}: image has zero size'.format(room_no, obj_i))
            continue

        #if obj_parent:
        #    print('Room {}, object {}: has parent {}'.format(room_no, obj_i, obj_parent))
        #    continue

        # print('decoding object image, size: {}'.format(obj_height*obj_width*3))
        obj_image = decodeRLE(room, obj_image_offset, obj_height*obj_width*3)
        obj_picmap = obj_image[0 : obj_height*obj_width]
        obj_colourmap = obj_image[obj_height*obj_width : 2*obj_height*obj_width]

        text_i = 0
        for row in range(obj_height):
            for column in range(obj_width):
                c = obj_picmap[text_i]
                colours[3] = obj_colourmap[text_i] & 0x07
                put_char(image, step, charmap, colours, (obj_x+column)*8, (obj_y+row)*8, c)
                text_i += 1

    write_ppm(image, width*8, height*8, 'room{:02d}o.ppm'.format(room_no))


with open('MANIAC1.D64', 'rb') as f:
    disk1 = f.read()

with open('MANIAC2.D64', 'rb') as f:
    disk2 = f.read()

# patch
if disk2[0x29fff] == 0x4b:
    disk2 = list(disk2)
    disk2[0x29fff] = 0x82
    disk2 = bytes(disk2)

start = index_start

(room_disks, room_offsets, start) = read_room_disks_and_offsets(disk1, start, num_rooms)

(costume_rooms, costume_offsets, start) = read_resource_rooms_and_offsets(disk1, start, num_costumes)
(script_rooms, script_offsets, start) = read_resource_rooms_and_offsets(disk1, start, num_scripts)
(sound_rooms, sound_offsets, start) = read_resource_rooms_and_offsets(disk1, start, num_sounds)

rooms = [ disk1[0:0x4a4] ] + [extract_resource(disk1, disk2, room_disks, room_offsets, room_no, 0) for room_no in range(1, 53)]

if __name__ == '__main__':

    #save_resources(disk1, disk2, room_disks, room_offsets, costume_rooms, costume_offsets, 'cos')
    #save_resources(disk1, disk2, room_disks, room_offsets, script_rooms, script_offsets, 'scr')
    #save_resources(disk1, disk2, room_disks, room_offsets, sound_rooms, sound_offsets, 'sou')

    for i in range(0, len(rooms)):
        with open('{:02d}.lfl'.format(i), 'wb') as f:
            f.write(rooms[i])

    print('Done')
