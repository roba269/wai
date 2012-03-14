from django.db import models

GAME_TYPE_CHOICES = (
    ('RENJU', 'simplest Renju'),
    ('OTHELLO', 'Othello'),
)

class User(models.Model):
    name = models.CharField(max_length=20)
    email = models.EmailField()
    passwd = models.CharField(max_length=100)
    
class Submit(models.Model):
    user = models.ForeignKey(User)
    game_type = models.CharField(max_length=10, choices=GAME_TYPE_CHOICES)
    lang = models.IntegerField()
    sub_time = models.DateTimeField()
    status = models.IntegerField()

class Match(models.Model):
    game_type = models.CharField(max_length=10, choices=GAME_TYPE_CHOICES)
    result = models.IntegerField()
    start_time = models.DateTimeField()
    end_time = models.DateTimeField()
    player_cnt = models.IntegerField()
    players = models.ManyToManyField(Submit)


