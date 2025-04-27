from pprint import pprint
from ctypes import *

class Point(Structure):
    _pack_ = 1
    _fields_ = [
        ('align',      c_uint8 ),
        ('vertical',   c_uint16),
        ('horizontal', c_uint16)
    ]

class Entry(Structure):
    _pack_ = 1
    _fields_= [
        ('point',  Point),
        ('fade_in',  c_float),
        ('fade_out', c_float),
        ('x',     c_int32),
        ('y',     c_int32),
        ('width', c_int32),
        ('height',c_int32),
    ]
class ImageEntry(Structure):
    _pack_ = 1
    _fields_ = [
        ('start',    c_float),
        ('end',      c_float),
        ('length',   c_uint16),
        ('entries',  POINTER(Entry))
    ]

def to_point(pos: str) -> dict[str, any]:
    pos: list[str] = pos.split(',')
    if len(pos) < 2:
        return None
    aligns: dict[str, int] = {
            'left'  : 1 << 0,
            'right' : 1 << 1,
            'top'   : 1 << 2,
            'bottom': 1 << 3,
            'center': 1 << 4,
            'middle': 1 << 5,
        }
    vertical: int = 0
    horizontal: int = 0
    align: int = 0
    if len(pos) == 2:
        value : int| None = to_integer(pos[1])
        if value is None:
            return None
        align_str: str = str(pos[0]).strip()
        if align_str not in aligns.keys():
            return None
        align = aligns[align_str]
        if align_str == 'left' or align_str == 'right':
            align = align | aligns['top']
        elif align_str == 'top' or align_str == 'bottom':
            align = align | aligns['left']
            pass
        vertical = value
        horizontal = value
        pass
    elif len(pos) == 3:
        value_1 : int| None = to_integer(pos[1])
        value_2 : int = 0
        align_1: str = str(pos[0]).strip()
        align_2: str = ''
        if value_1 is None:
            value_1 = to_integer(pos[2])
            if value_1 is None: return None
            value_2 = value_1
            align_2 = str(pos[1]).strip()
        else:
            value_2 = to_integer(pos[2])
            if value_2 is None: 
                return None
            pass
        if align_1 not in aligns.keys():
            return None
        align = aligns[align_1]
        if len(align_2) != 0:
            if align_2 not in aligns.keys():
                return None
            align = align | aligns[align_2]
        else:
            if align_1 == 'left' or align_1 == 'right':
                align = align | aligns['top']
            elif align_1 == 'top' or align_1 == 'bottom':
                align = align | aligns['left']
            pass
        pass
        vertical = value_1
        horizontal = value_2
    elif len(pos) == 4:
        align_1: str = str(pos[0]).strip()
        align_2: str = str(pos[1]).strip()
        value_1: str = str(pos[2]).strip()
        value_2: str = str(pos[3]).strip()
        if align_1 not in aligns.keys():
            return None
        if align_2 not in aligns.keys():
            return None
        vertical = to_integer(value_1)
        if vertical is None:
            return None
        horizontal = to_integer(value_2)
        if horizontal is None:
            return None
        align = aligns[align_1]
        align = align | aligns[align_2]
    else:
        return None
    return {
        "align": align,
        "vertical": vertical,
        "horizontal": horizontal
    }

def to_integer(value: str) -> int | None:
    try:
        return int(str(value).strip())
    except:
        return None

def time_to_float(time: str) -> float | None:
    parts: list[str] = time.split(':')
    try:
        if len(parts) == 3:
            hours = float(parts[0])
            minutes = float(parts[1])
            seconds = float(parts[2])
            total_seconds = hours * 3600 + minutes * 60 + seconds
            return round(total_seconds, 3)
        elif len(parts) == 2:
            minutes = float(parts[0])
            seconds = float(parts[1])
            total_seconds = minutes * 60 + seconds
            return round(total_seconds, 3)
        elif len(parts) == 1:
            return round(float(parts[0]), 3)
    except:
        pass
    return None

def parse_makesub(lines: list[str]) -> dict[str, any]:
    result: dict[str, any] = { "name": "", "fade": "", "entries": [] }
    values: dict[str, any] = {}
    sub: dict[str, str] = {}
    flag: int = 0;
    for line in lines:
        line = line.strip()
        if len(line) == 0:
            continue
        if line.startswith(".name"):
            name: str = line[5:].strip()
            result['name'] = name
            values['name'] = name
            continue
        if line.startswith(".fade"):
            # result['fade'] = fade
            values['fade'] = line[5:].strip()
            continue
        if line.startswith(".width"):
            width: int = int(line[6:].strip())
            result['width'] = width
            values['width'] = width
            continue
        if line.startswith(".height"):
            height: int = int(line[7:].strip())
            result['height'] = height
            values['height'] = height
            continue
        if line.startswith(":image-sub"):
            flag = 1
            sub = {}
            continue
        if line.startswith(":end"):
            if sub:
                time: str = sub.get('time')
                for n, sub in sub.items():
                    if 'time' == n:
                        continue
                    sub['time'] = time
                    result["entries"].append(sub)
            flag = 0
            continue
        if flag == 1 and line.startswith('.'):
            args: list[str] = line.split(' ', 1)
            if len(args) < 2:
                continue
            key: str = str(args[0][1:]).strip()
            value: str = str(args[1]).strip()
            if key != 'time':
                args: list[str] = value.split(' ')
                value: dict[str, str] = { "name": args[0] }
                if len(args) > 1:
                    args: list[str] = [
                            str(arg).strip() 
                            for arg in args[1:] 
                            if len(str(arg).strip()) > 0
                        ]
                    args: str = ''.join(args)
                    current: int = 0
                    while True:
                        name: str = ''
                        begin: int = args.find('(', current)
                        if begin == -1:
                            break;
                        name = args[current: begin]
                        current = begin
                        end: int = str(args).find(')', current)
                        if end == -1:
                            break;
                        current = end + 1
                        arg: str = args[begin + 1: end]
                        value[name] = arg
                    pass
                if value.get('pos') is None:
                    pos = values.get(f"{key}::pos")
                    if pos is None:
                        pos = values.get("pos")
                    if pos is not None:
                        value['pos'] = pos
                if value.get('fade') is None:
                    fade = values.get(f"{key}::fade")
                    if fade is None:
                        fade = values.get("fade")
                    if fade is not None:
                        value['fade'] = fade
                # print(pos)
                pass
            sub[key] = value
            continue
        if line.startswith('.'):
            args: list[str] = line.split(" ")
            key: str = str(args[0][1:]).strip()
            value: str = ''.join(args[1:]).strip()
            values[key] = value
            continue
    return result

def read_makesub(path: str) -> dict[str, any]:
    with open(path, mode='r', encoding='utf-8') as f:
        return parse_makesub(f.readlines())
    return {}

def make_sub_file(info: dict[str, any], input_path: str) -> any:
    import os
    from PIL import Image
    width: int = info.get('width')
    height: int = info.get('height')
    total_width: int = 0
    max_height:  int = 0
    image_entries: list[dict[str, any]] = []
    image_list: dict[str, any] = {}
    for entry in info.get('entries'):
        name: str = entry.get('name')
        if name is None:
            continue

        time: str = entry.get('time')
        if time is None:
            continue
        
        time: list[str] = [t for t in time.split(',') if len(str(t).strip()) > 0]
        if len(time) != 2:
            continue
        
        start_time: float|None = time_to_float(time[0])
        if start_time is None:
            continue
        end_time: float|None = time_to_float(time[1])
        if end_time is None:
            continue

        # print(f"start_time: {start_time} end_time: {end_time}")

        pos: str = entry.get('pos')
        if pos is None:
            pos = 'top,left,0,0'

        point: dict[str, any] = to_point(pos)
        if point is None:
            continue

        fade_in : float = 0.0
        fade_out: float = 0.0
        fade: str = entry.get('fade')
        if fade is not None:
            fade: list[str] = fade.split(',')
            if len(fade) == 2:
                try:
                    fade_in = round(float(str(fade[0]).strip()), 3)
                except:
                    pass
                try:
                    fade_out = round(float(str(fade[1]).strip()), 3)
                except:
                    pass
            elif len(fade) == 1:
                try:
                    fade_in = round(float(str(fade[0]).strip()), 3)
                except:
                    pass
                fade_out = fade_in
            pass
        img_entry = {
            "point":   point,
            "fadein":  fade_in,
            "fadeout": fade_out,
            "x": 0,
            "y": 0,
            "width":  0,
            "height": 0
        }
        image_sub =  {
            "start": start_time,
            "end": end_time,
            "entries": []
        }
        index: int = -1
        for i in range(0,len(image_entries)):
            if image_entries[i]['start'] == start_time and image_entries[i]['end'] == end_time:
                image_sub['entries'] = image_entries[i]['entries']
                index = i
                break
            pass
        try:
            file: str = os.path.join(input_path, name)
            if file in image_list.keys():
                image = image_list[file]
                img_entry['width']  = image["width"]
                img_entry['height'] = image["height"]
                img_entry['x'] = image["x"]
                img_entry['y'] = image["y"]
            else:
                img = Image.open(file)
                img_entry['width']  = img.size[0]
                img_entry['height'] = img.size[1]
                img_entry['x'] = total_width
                total_width += img.size[0]
                max_height = max(max_height, img.size[1])
                image_list[file] = {
                    "width" : img_entry["width"],
                    "height": img_entry["height"],
                    "x": img_entry['x'],
                    "y": img_entry['y'],
                    "img" : img
                }
        except Exception as err:
            continue
        image_sub['entries'].append(img_entry)
        if index != -1:
            image_entries[index] = image_sub
        else:
            image_entries.append(image_sub)
        # print(entry)
        pass

    sprite_sheet = Image.new('RGBA', (total_width, max_height), (0, 0, 0, 0))
    total_width = 0
    for image in image_list.values():
        image = image['img']
        sprite_sheet.paste(image, (total_width, 0))
        total_width += image.size[0]
        pass
    from io import BytesIO
    sprite_data = BytesIO()
    sprite_sheet.save(sprite_data, format='png')

    entries_data = BytesIO()
    for entry in image_entries:
        # pprint(entry)
        _entry = ImageEntry()
        _entry.start  = entry['start']
        _entry.end    = entry['end']
        _entry.length =  len(entry['entries'])
        size = sizeof(ImageEntry) - sizeof(POINTER(Entry))
        data = string_at(byref(_entry), size)
        entries_data.write(data)
        for i in range(_entry.length):
            tmp = entry['entries'][i]
            _entry = Entry()
            _point = Point()
            _point.align      = tmp['point']['align']
            _point.vertical   = tmp['point']['vertical']
            _point.horizontal = tmp['point']['horizontal']
            _entry.point = _point
            _entry.fade_in = tmp['fadein']
            _entry.fade_out = tmp['fadeout']
            _entry.x = tmp['x']
            _entry.y = tmp['y']
            _entry.width = tmp['width']
            _entry.height = tmp['height']
            data = string_at(byref(_entry), sizeof(Entry))
            entries_data.write(data)
    name: str = info.get('name')
    if name is None:
        import hashlib
        hash_object = hashlib.md5()
        hash_object.update(data)
        name = hash_object.hexdigest()

    with open(f"{name}.xsub", "wb") as f:
        f.write(b'xsub')
        f.write(b'\x01\x00\x00\x00')
        f.write(string_at(byref(c_uint16(width)), sizeof(c_uint16)))
        f.write(string_at(byref(c_uint16(height)), sizeof(c_uint16)))
        entries_data = entries_data.getvalue()
        f.write(string_at(byref(c_uint32(len(entries_data))), sizeof(c_uint32)))
        f.write(entries_data)
        f.write(sprite_data.getvalue())
    print(f"saved: {name}.xsub")
    pass

if __name__ == '__main__':
    makesub = read_makesub("./271/.makesub")
    make_sub_file(makesub, './271/')
    pass