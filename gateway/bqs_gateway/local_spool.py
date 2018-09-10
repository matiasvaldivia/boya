import sqlite3
import json
import time

class LocalSpoolQueue:
    def __init__(self, db_path="spool.db"):
        self.db_path = db_path
        self.init_db()

    def init_db(self):
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS outgoing_telemetry (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    node_id INTEGER NOT NULL,
                    sequence INTEGER NOT NULL,
                    payload_json TEXT NOT NULL,
                    created_at INTEGER NOT NULL,
                    retries INTEGER DEFAULT 0,
                    status TEXT DEFAULT 'PENDING',
                    UNIQUE(node_id, sequence)
                )
            """)
            conn.commit()

    def push(self, telemetry_dict: dict) -> bool:
        node_id = telemetry_dict.get("node_id")
        seq = telemetry_dict.get("sequence")
        payload = json.dumps(telemetry_dict)
        now = int(time.time())

        try:
            with sqlite3.connect(self.db_path) as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    INSERT OR IGNORE INTO outgoing_telemetry (node_id, sequence, payload_json, created_at)
                    VALUES (?, ?, ?, ?)
                """, (node_id, seq, payload, now))
                conn.commit()
                return True
        except Exception:
            return False

    def get_pending(self, limit=10) -> list:
        results = []
        try:
            with sqlite3.connect(self.db_path) as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    SELECT id, node_id, sequence, payload_json, retries
                    FROM outgoing_telemetry
                    WHERE status = 'PENDING'
                    ORDER BY id ASC
                    LIMIT ?
                """, (limit,))
                for row in cursor.fetchall():
                    results.append({
                        "id": row[0],
                        "node_id": row[1],
                        "sequence": row[2],
                        "payload": json.loads(row[3]),
                        "retries": row[4]
                    })
        except Exception:
            pass
        return results

    def mark_success(self, record_id: int):
        try:
            with sqlite3.connect(self.db_path) as conn:
                cursor = conn.cursor()
                cursor.execute("DELETE FROM outgoing_telemetry WHERE id = ?", (record_id,))
                conn.commit()
        except Exception:
            pass

    def increment_retry(self, record_id: int):
        try:
            with sqlite3.connect(self.db_path) as conn:
                cursor = conn.cursor()
                cursor.execute("UPDATE outgoing_telemetry SET retries = retries + 1 WHERE id = ?", (record_id,))
                conn.commit()
        except Exception:
            pass
