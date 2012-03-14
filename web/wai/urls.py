from django.conf.urls.defaults import patterns, include, url

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
        url(r'^login/', 'wai.views.login'),
        url(r'^logout/', 'wai.views.logout'),
        url(r'^submit/', 'wai.views.submit'),
        url(r'^match_list/', 'wai.views.show_match_list'),
        url(r'^admin/', include(admin.site.urls)),
        url(r'^$', 'wai.views.home'),
    # Examples:
    # url(r'^$', 'wai.views.home', name='home'),
    # url(r'^wai/', include('wai.foo.urls')),

    # Uncomment the admin/doc line below to enable admin documentation:
    # url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

    # Uncomment the next line to enable the admin:
)
