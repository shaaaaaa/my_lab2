import mysql.connector

userDbConfig = {
    "host": "10.107.1.175",
    "port": 3306,
    "database": "user",
    "user": "pavi",
    "password": "Platform1345",
}

tidbConfig={
    "host": "10.107.1.175",
    "port": 3306,
    "database": "user",
    "user": "pavi",
    "password": "Platform1345",
}

'''
with mysql.connector.connect(**userDbConfig) as userConn:
    with userConn.cursor(dictionary=True) as userCur:
        userCur.execute("SELECT id,username,created FROM user WHERE id=%s",["1000testt"])
        users=userCur.fetchall()
        userCur.execute(
            "INSERT INTO user2 (id,username,created) VALUES(%s,%s,%s)",
            ["test333","test333","2019-10-21 21:58:19"])
        print(userCur.rowcount)
        for user in users:
            print(user)
        userCur.execute(
            "INSERT INTO user2 (id,username,created) VALUES(%s,%s,%s)",
            ["test222","test222","2019-10-21 21:58:19"])
        userConn.commit()
'''
        
            


with mysql.connector.connect(**userDbConfig) as userConn,mysql.connector.connect(**tidbConfig) as tidbConn:
    with userConn.cursor(dictionary=True) as userCur,tidbConn.cursor() as tidbCur:
        userCur.execute("SELECT id,username,created FROM user WHERE usertype=1")
        users = userCur.fetchall()
        for user in users:
            tidbCur.execute("SELECT * FROM user2 WHERE id=%s", [user["id"]])
            tidbCur.fetchall()
            if tidbCur.rowcount > 0:
               continue
            tidbCur.execute(
                "INSERT INTO user2 (id,username,created) VALUES(%s,%s,%s)",
                [user["id"],user["username"],user["created"]])
        tidbConn.commit()


