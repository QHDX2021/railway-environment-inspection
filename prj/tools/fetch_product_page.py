"""Fetch a public product page without browser cookies or authentication."""
import json
import re
from datetime import datetime, timezone
from html.parser import HTMLParser
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


class PageParser(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.hidden = 0
        self.in_title = False
        self.title = []
        self.text = []
        self.meta = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag in ('script', 'style'):
            self.hidden += 1
        if tag == 'title':
            self.in_title = True
        if tag == 'meta' and attrs.get('content'):
            self.meta.append(attrs)

    def handle_endtag(self, tag):
        if tag in ('script', 'style'):
            self.hidden = max(0, self.hidden - 1)
        if tag == 'title':
            self.in_title = False

    def handle_data(self, data):
        if self.in_title:
            self.title.append(data.strip())
        if not self.hidden and data.strip():
            self.text.append(data.strip())


def main():
    url = 'https://detail.tmall.com/item.htm?id=718486986814&skuId=5650971165647'
    output = Path(__file__).resolve().parents[1] / 'web_capture' / 'tmall_718486986814'
    output.mkdir(parents=True, exist_ok=True)
    report = {'requested_url': url, 'fetched_at_utc': datetime.now(timezone.utc).isoformat(),
              'method': 'Public HTTP GET; no cookies or authentication supplied'}
    request = Request(url, headers={'User-Agent': 'Mozilla/5.0',
                                   'Accept': 'text/html,application/xhtml+xml',
                                   'Accept-Language': 'zh-CN,zh;q=0.9'})
    try:
        try:
            response = urlopen(request, timeout=25)
        except HTTPError as exc:
            response = exc
        with response:
            raw = response.read(8 * 1024 * 1024 + 1)
            report.update(status=response.status, final_url=response.url,
                          content_type=response.headers.get('Content-Type', ''),
                          bytes_read=len(raw))
            if len(raw) > 8 * 1024 * 1024:
                raise ValueError('Response exceeds 8 MiB capture limit')
            charset = response.headers.get_content_charset()
            if not charset:
                match = re.search(br'charset\s*=\s*["\x27]?([a-zA-Z0-9_-]+)', raw[:8192])
                charset = match.group(1).decode('ascii') if match else 'utf-8'
            html = raw.decode(charset, errors='replace')
            (output / 'response.html').write_text(html, encoding='utf-8')
            parser = PageParser()
            parser.feed(html)
            report.update(encoding=charset, title=' '.join(parser.title), meta=parser.meta)
            (output / 'visible_text.txt').write_text('\n'.join(parser.text), encoding='utf-8')
            # Keyword hits are diagnostic snippets, not verified product attributes.
            hits = []
            for match in re.finditer(r'718486986814|5650971165647|Jetson|Orin|16GB|16G|基础套餐|登录|验证|captcha', html, re.I):
                hits.append(html[max(0, match.start()-80):match.end()+160])
                if len(hits) >= 35:
                    break
            (output / 'keyword_snippets.json').write_text(json.dumps(hits, ensure_ascii=False, indent=2), encoding='utf-8')
            report['visible_text_preview'] = '\n'.join(parser.text)[:5000]
            report['keyword_hit_count_capped_35'] = len(hits)
    except (URLError, OSError, ValueError, LookupError) as exc:
        report['error'] = str(exc)
    (output / 'fetch_report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
