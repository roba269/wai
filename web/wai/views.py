from django.http import HttpResponse
from django.template import RequestContext
from django.shortcuts import render_to_response
from main_app.models import User

def home(request):
    return HttpResponse("Hello world!\n")

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
            return HttpResponse("hello" + u.name)
        else:
            return HttpResponse("passwd error!")
    except:
        return HttpResponse("User Doesn't exist")

