from django.http import HttpResponse, HttpResponseRedirect
from django.template import RequestContext
from django.shortcuts import render_to_response
from main_app.models import User, Match, Submit
import datetime

SRC_PATH = '/home/roba/wai/submit/'

def home(request):
    login_user = request.session.get('user', None)
    if login_user == None:
        return HttpResponse("hello world!\n")
    return HttpResponse("Hello " + login_user)

def login(request):
    try:
        input_user = request.POST['user']
        input_passwd = request.POST['passwd']
    except (KeyError):
        # no POST field, as normal page
        return render_to_response('login.html', context_instance = RequestContext(request))
    try:
        u = User.objects.get(name=input_user)
        if u.passwd == input_passwd:
            request.session['user'] = input_user
            return HttpResponse("hello" + u.name)
        else:
            return HttpResponse("passwd error!")
    except:
        return HttpResponse("User Doesn't exist")

def logout(request):
    try:
        del request.session['user']
    except:
        pass
    return HttpResponse("logout success")

def submit(request):
    if not ('user' in request.session):
        return HttpResponseRedirect('/login/')
    if request.method == 'GET':
        return render_to_response('submit.html',
                context_instance = RequestContext(request))
    elif request.method == 'POST':
#        try:
            cur_user = User.objects.get(name = request.session['user'])
            sub_lang = int(request.POST['lang'])
            new_sub = Submit(user = cur_user, game_type = 'RENJU',
                    lang = sub_lang,
                    sub_time = datetime.datetime.now(),
                    status = 0)
            new_sub.save()
            sub_id = new_sub.id
            ext = ''
            if sub_lang == 0: ext = '.cpp'
            fp = open('%s%d%s' % (SRC_PATH, sub_id, ext), 'w')
            fp.write(request.POST['src'])
            fp.close()
            return HttpResponse('Submit successfully')
# except:
#            return HttpResponse('Exception!')

def show_match_list(request, game_type):
    match_list = Match.objects.filter(game_type__exact=game_type).order_by('-start_time')
    return render_to_response('match_list.html', {'match_list': match_list})

def show_submit_list(request, game_type, uid):
    sub_list = Submit.objects.filter(game_type__exact=game_type).filter(user__exact=uid).order_by('-sub_time')
    return render_to_response('sub_list.html', {'sub_list': sub_list})

def show_source(request, sub_id):
    fp = open('/home/roba/wai/submit/' + sub_id + '.cpp', 'r')
    ss = fp.read()
    fp.close()
    return HttpResponse(ss)

def show_record(request, match_id):
    fp = open('/home/roba/wai/record/' + match_id + '.txt', 'r')
    ss = fp.read()
    fp.close()
    js_array = ''
    for tmp in ss.split(','):
        try:
            tmp_color = int(tmp.split(':')[0])
            tmp_x = int(tmp.split(':')[1].split(' ')[0])
            tmp_y = int(tmp.split(':')[1].split(' ')[1])
            if len(js_array): js_array += ','
            js_array += '{x:' + str(tmp_x) + ',y:' + str(tmp_y) + ',color:' + str(tmp_color) + '}'
        except:
            break
    return render_to_response('renju.html', {'record_str': js_array})

