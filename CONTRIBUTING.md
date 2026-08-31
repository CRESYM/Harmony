# Harmony Contributing Guidelines

Thank you for considering making a contribution to Harmony! This document explains how to make changes to the project, whether you are adding a new feature, fixing a bug, improving the documentation or doing something else. 

This document contains the following sections:
- [Internal Contributors](#internal-contributors) - procedure for contributors who have access to the Harmony repository.
- [External Contributors](#external-contributors) - procedure for contributing through a fork of the Harmony repository.
- [Git Flow Description](#git-flow) – overview of the branching workflow used by Harmony.
- [Frequently Asked Questions](#frequently-asked-questions) - solutions to common Git problems and links to further resources.

## Internal Contributors

The following procedure is for contributors who are members of the Harmony team or a related project team.

### 1. Request access to the repository

Ask a current Harmony team member to add you as a collaborator to the Harmony repository. You only need to do this once. If you already have access to the repository, continue to the next step.

### 2. Create an issue

Before starting to work, create an issue describing what you intend to change. You can create an issue using the [Issues tab](https://github.com/CRESYM/Harmony/issues) at the top of the repository.

Even if you have already discussed the proposed changes with the team, it is useful to document them in the repository. This gives other contributors and future maintainers a record of why the change was made and what it was intended to accomplish. If the change has already been discussed by the team, you can briefly mention that in the issue.

### 3. Set up Hamony locally

Follow the instructions in the README to set up a local working version of Harmony. Once it is setup, compile the application, run the tests and the examples to make sure everything works before you start making changes. This gives you a known working starting point. If something does not work at this stage, it is much easier to identify the problem before making your own changes.

### 4. Create a branch

Changes to Harmony should be made in a separate branch rather than directly in `develop`.

First, make sure you are starting from the latest version of `develop`. See [Git Flow](#git-flow) section below for instructions.

Give your branch a name that includes the issue number and a short description of the change. For example, if issue `166` is about adding a new solver called `foo`, you could name the branch `feature/166-add-foo-solver`. If the issue is about fixing a problem with the solver, you can name the branch `bugfix/166-fix-foo-solver`. The exact wording is not important. The goal is to make it easy for other contributors to understand what the branch is for.

### 5. Make your changes

Make the required changes in your branch. Try to keep your changes focused on the issue you are working on. If you notice an unrelated problem while working, consider creating a separate issue for it rather than including it in the same change.

Save your work regularly by making commits. A commit is simply a saved step in the history of your work. When creating a commit, write a short message describing the changes included in it.

You may also want to consult the [developer guide](docs/developer-guide.md) which explains how to carry out common development tasks.

### 6. Run the tests

Before asking for your changes to be reviewed, compile Harmony and run the tests and examples again. If you have added or changed functionality, add or update tests where appropriate. Tests help make sure that the change works as intended and that existing functionality has not been accidentally broken.

If a test fails, try to understand whether the failure is caused by your changes before opening the pull request. If you are unsure, mention the failure in the pull request so that another contributor can help.

### 7. Push your branch to GitHub

Push your branch to the Harmony GitHub repository.

You can push your branch at any point while you are working. This is useful even before the work is complete because other team members can see your progress and help if you run into problems.

If you continue working on the branch after pushing it, remember to push your new commits as well.

### 8. Open a pull request 

When your changes are ready for review, open a pull request to merge your branch into `develop`. 

A pull request is a request for the team to review your changes before they become part of the shared development version of Harmony.

When creating the pull request:
- Give it a clear title describing the change.
- Briefly explain what you changed and why.
- Link the issue you created in Step 2. You can do this using the menu on the right-hand side of the pull request page.
- Add at least one member of the Harmony team as a reviewer.
- Mention anything that the reviewer should pay particular attention to.
- If there are known limitations or tests that you could not run, mention them.

### 9. Address review comments

If the reviewer asks you to make changes, make those changes in your branch and push them to GitHub. The pull request will automatically be updated with your new changes. There is no need to create a new pull request.

Once the reviewers are satisfied, the branch can be merged into `develop`. After the pull request has been merged, your feature/bugfix branch is no longer needed and can be deleted.

If you linked the issue created in Step 2 to the Pull Request, the issue will be closed when the pull request is closed and your branch has been merged.


## External Contributors

The procedure for external contributors is very similar to the procedure for [internal contributors](#internal-contributors). 

The main difference is that external contributors do not have access to make changes directly to the Harmony repository. Instead, you will create your own copy of the Harmony repository, called a **fork**, and make your changes there. When your changes are ready, you can submit them to Harmony through a pull request.

Follow the procedure for internal contributors, with the following differences:


### 1. Fork the Harmony repository
Instead of requesting access to the Harmony repository, create a fork of it on GitHub.
1. Go to the Harmony repository.
2. Click the Fork button (top-right).
3. Select your GitHub account as the owner of the fork.
4. If GitHub shows the option **Copy the DEFAULT branch only**, make sure it is **NOT selected**. This ensures that the `develop` branch is also copied to your fork.
5. Click Create fork.

You now have your own copy of Harmony. Clone this repository to your computer and follow the setup instructions in the README.

### 2. Create your branch, work on your fork and submit your changes

Follow Step 2 and Step 3 of the Internal Contributors procedure as usual. When you are ready to create your branch, first make sure that your local `develop` branch contains the latest changes from the Harmony repository. See next step for instructions on how to do this.

Work on your branch following the remaining steps in the Internal Contributions procedure. When your changes are ready, open a pull request **from your feature/bugfix branch in your fork to the `develop` branch in the Harmony repository**.

### 3. Keep your branch up to date

If changes are made to Harmony's `develop` branch while you are working, update your fork:
1. Open your fork repository on GitHub.
2. Select the develop branch.
3. Click Sync Fork and select Update Branch.

This will add all of the new commits in Harmony's `develop` branch to the `develop` branch in your fork. Follow the instructions in the [Keeping your branch up to date](#keeping-your-branch-up-to-date) section below to learn how to add these commits to your feature or bugfix branch.

Read more in [GitHub's guide for syncing a fork](https://docs.github.com/en/pull-requests/how-tos/work-with-forks/syncing-a-fork).

## Git Flow

Changes to Harmony should follow the **Git Flow** workflow which keeps the stable version of Harmony separate from work that is still being developed. The workflow involves three types of branches:
* `main` - contains stable versions of Harmony. This branch should only contain changes that are ready for a release. **Do not make changes directly to main.**
* `develop` - contains changes that are being developed and will eventually become part of a stable release.
* `feature/bugfix/change branches` - temporary branches used to work on a specific feature, bug fix, or other change. These branches are created from `develop` and are merged back into `develop` when the work is complete.

### Creating a new branch

When you start working on an issue, create a new branch from the latest commit in the `develop` branch. You can do so by running the following commands from the terminal:

```bash
# Navigate to the harmony repository
cd <path-to-harmony>

# Switch to the develop branch
git checkout develop

# Retrieve the latest commits from GitHub
git pull

# Create a new branch and switch to it.
# Replace 'feature/166-add-foo-solver' with the name of your own branch
git checkout -b feature/166-add-foo-solver
```

You can now make your changes without modifying the `develop` branch.

> [!TIP]
> You can check which branch is currently active with `git status`. You can switch between branches with `git checkout <branch-name>`.

### Working on your branch

As you work, save your changes by creating commits:

```bash
# Add all the files you have modified and that you wish to include in the commit
git add <file-1> <file-2> <file-n>

# Create a commit 
git commit -m "Add foo solver"

# Push your branch to GitHub
git push -u origin feature/166-add-foo-solver
```

You can continue to make commits and push them to the same branch while your work.

> [!TIP]
> Use `git status` to list all the files that have changed since the last commit. Use `git diff <file-name>` to display the changes in a specific file.

### Keeping your branch up to date
Other contributors may make changes to `develop` while you are working. It is a good idea to occasionally update your branch with the latest changes from `develop`.

First, make sure your own changes have been committed. Then, update your local copy of develop with the latest changes from GitHub and merge `develop` into your own branch:

```bash
# Switch to the develop branch
git checkout develop

# Pull the latest changes from GitHub
git pull

# Switch back to your branch
# Replace '166-add-foo-solver' with the name of your own branch
git checkout feature/166-add-foo-solver

# Merge the latest changes from develop into your branch
git merge develop
```

If the merge is successful, you can continue working on your branch. You may want to run the tests to make sure everything still works. If Git reports merge conflicts, see "How do I solve merge conflicts between branch A and B?" in the [FAQ section](#frequently-asked-questions) below.

Once you have resolved any conflicts and confirmed that everything works, push the updated branch to GitHub using `git push`.

### Completing your work

When your changes are ready:
1. Make sure your changes are committed and pushed to GitHub.
2. Compile, run the tests and examples
3. Open a pull-request to merge your feature/bugfix branch into `develop`
4. Ask a Harmony team member to review the pull-request.
5. Address any review comments.
6. Once the pull request is approved, it can be merged into `develop`.

The `main` branch is reserved for stable releases and should not be used for day-to-day development.


## Frequently Asked Questions

### I cannot create a new branch because I have local uncommitted changes. What can I do?
Git may prevent you from switching branches or creating a new branch when you have changes that have not been saved in a commit.

First, check what has changed:
```bash
# List the names of files that have changes
git status

# Display the changes on a particular file
git diff <file-name>

# Display the chanegs on all files
git diff
```

If you want to keep the changes, the safest option is usually to commit them before switching branches:
```bash
git add <file-1> <file-2> <file-n>
git commit -m "Save my current work"
```

### How do I solver merge conflicts between branch A and B?
A merge conflict happens when Git cannot automatically decide which version of a change should be kept.

This can happen when two branches have changed the same part of a file.

If you encounter a merge conflict:
1. Do not panic.
2. Check which files Git reports as having conflicts.
3. Open those files and look for the sections marked by Git with `<<<<<<<`, `=======`, `>>>>>>>`.
4. Decide which changes should be kept, or combine the changes if both are needed.
5. Remove the conflict markers added by Git.
6. Compile and run the tests after resolving the conflict.
7. Commit the resolved changes.

For more information on how to solve git conflicts see
* [Code Refinery - lesson on conflict resolution](https://coderefinery.github.io/git-intro/conflicts/)
* [GitHub: resolving a merge conflict](https://docs.github.com/en/pull-requests/how-tos/merge-and-close-pull-requests/resolving-a-merge-conflict-on-github)


### Where can I learn more about git?

There are many free resources online to learn about Git. We find the following ones particularly useful:
* [GitHub - GitHub flow](https://docs.github.com/en/get-started/using-github/github-flow)
* [The Carprenties - Version Control with Git](https://swcarpentry.github.io/git-novice/)
* [CodeRefinery - Introduction to version control with Git](https://coderefinery.github.io/git-intro/)

Other resources:
* [TU Delft Library](https://www.tudelft.nl/en/library/data-management/training-and-events/training-for-researchers) offers a selection of courses aimed at TU Delft researchers.
* [4TU](https://community.data.4tu.nl/introduction-to-version-control-with-git/) offers various introductory-level courses including _Introduction to Version Control with Git_.
* [NL eScience Centre](https://www.esciencecenter.nl/digital-skills/) offers a selection of introductory and advanced level courses on different topics.
* [TU Delft Digital Competence Centre](https://dcc.tudelft.nl) offers a _Code & Data Office Hours_ service to get help on a specific topic; also hosts the _DCC Guides_ which provide practical information on research software management.