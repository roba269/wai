from django.db import models

class User(models.Model):
    name = models.CharField(max_length=20)
    email = models.EmailField()
    passwd = models.CharField(max_length=100)
    
class Match(models.Model):
    result = models.IntegerField()

class Submit(models.Model):
    user = models.ForeignKey(User)
    match = models.ForeignKey(Match)
    lang = models.IntegerField()
    sub_time = models.DateTimeField()
    status = models.IntegerField()

