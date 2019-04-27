#!/usr/bin/env python
'''
This script uses GitHub API to construct http_archive element to be inserted
into a federation client projects bazel WORKSPACE
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


PROJECTS = [
    GitHubProject('com_google_absl_oss_federation',
      'abseil', 'federation-head'),
]

for project in PROJECTS:
  retVal=project.workspace_rule()
  print ("********** INSERT THIS INTO YOUR WORKSPACE: *****************")
  print (retVal)
  print ("*********************************")

