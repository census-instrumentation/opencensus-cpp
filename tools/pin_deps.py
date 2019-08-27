#!/usr/bin/env python
'''
This script uses the GitHub API to construct http_archive elements to be
inserted into the WORKSPACE file.

It is a fork of:
https://github.com/abseil/federation-hello/blob/0c1ef91b9cc5aabf15a3ff15761837d1c8da93a6/head_sync.py
'''
import hashlib
import json
import urllib3

HTTP_ARCHIVE_TEMPLATE = """http_archive(
  name = "{}",
  urls = ["{}"],  # {}
  strip_prefix = "{}-{}",
  sha256 = "{}",
)"""

http = urllib3.PoolManager()


class ExternalDependency(object):
  def workspace_rule(self):
    raise NotImplementedError('must implement workspace_rule()')


class GitHubProject(ExternalDependency):
  def __init__(self, name, owner, repo):
    self.name = name
    self.owner = owner
    self.repo = repo

  def workspace_rule(self):
    # https://developer.github.com/v3/repos/commits/
    request = http.request(
        'GET', 'https://api.github.com/repos/{}/{}/commits'.format(
            project.owner, project.repo),
        headers = { 'User-Agent': 'Workspace Updater' })
    response = json.loads(request.data.decode('utf-8'))
    commit = response[0]["sha"]
    date = response[0]["commit"]["committer"]["date"]
    url = 'https://github.com/{}/{}/archive/{}.zip'.format(
        project.owner, project.repo, commit)
    request = http.request('GET', url,
                           headers = { 'User-Agent': 'Workspace Updater' })
    sha256 = hashlib.sha256(request.data).hexdigest()
    return HTTP_ARCHIVE_TEMPLATE.format(project.name, url, date, project.repo,
                                        commit, sha256)


# grep -A1 http_archive\( WORKSPACE
PROJECTS = [
    GitHubProject('rules_cc', 'bazelbuild', 'rules_cc'),
    GitHubProject('com_google_absl', 'abseil', 'abseil-cpp'),
    GitHubProject('com_google_googletest', 'google', 'googletest'),
    GitHubProject('com_github_google_benchmark', 'google', 'benchmark'),
    GitHubProject('com_github_grpc_grpc', 'grpc', 'grpc'),
    GitHubProject('com_github_jupp0r_prometheus_cpp', 'jupp0r', 'prometheus-cpp'),
    GitHubProject('com_github_curl', 'curl', 'curl'),
    GitHubProject('com_github_tencent_rapidjson', 'Tencent', 'rapidjson'),
    GitHubProject('com_google_googleapis', 'googleapis', 'googleapis'),
    # io_bazel_rules_go is pinned.
    GitHubProject('grpc_java', 'grpc', 'grpc-java'),
    GitHubProject('opencensus_proto', 'census-instrumentation', 'opencensus-proto'),
]

for project in PROJECTS:
  print(project.workspace_rule())
